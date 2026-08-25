#include "yoloseg.h"

#include <algorithm>
#include <cstring>
#include <cmath>
#include <exception>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

// 匿名命名空间，将内部的的常量限定在当前的编译单元。
namespace {
constexpr int kPersonClassId = 0;   // constexpr 常量表达式，表示在编译时就可以确定值

float sigmoid(float value)
{
    return 1.0f / (1.0f + std::exp(-value));
}

float clampFloat(float value, float low, float high)    //限定返回值范围
{
    return std::max(low, std::min(value, high));
}
}

// RAII（资源获取时处理化） 写法
// : m_env(... 语法：初始化列表，在进入大括号之前执行，想较与“=”，少了一次构造与析构开销
yoloSeg::yoloSeg()
    : m_env(ORT_LOGGING_LEVEL_WARNING, "RealSenseSnap.YoloSeg")
{
    m_sessionOptions.SetIntraOpNumThreads(1);
    m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
}

yoloSeg::yoloSeg(const std::string &modelPath)
    : yoloSeg()
{
    loadModel(modelPath);
}

bool yoloSeg::loadModel(const std::string &modelPath)
{
    try {
        m_modelPath = modelPath;
        // 智能指针，避免忘记delete、重复delete等问题
        m_session = std::make_unique<Ort::Session>( // 智能指针构造
            m_env,
            m_modelPath.c_str(),
            m_sessionOptions);

        Ort::AllocatorWithDefaultOptions allocator;
        m_inputNames.clear();
        m_outputNames.clear();

        const size_t inputCount = m_session->GetInputCount();
        for (size_t i = 0; i < inputCount; ++i) {
            auto name = m_session->GetInputNameAllocated(i, allocator);
            m_inputNames.emplace_back(name.get());
        }

        const size_t outputCount = m_session->GetOutputCount();
        for (size_t i = 0; i < outputCount; ++i) {
            auto name = m_session->GetOutputNameAllocated(i, allocator);
            m_outputNames.emplace_back(name.get());
        }

        auto inputInfo = m_session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo(); 
        m_inputShape = inputInfo.GetShape();
        if (m_inputShape.size() == 4) {
            if (m_inputShape[2] > 0) {
                m_inputHeight = static_cast<int>(m_inputShape[2]);
            }
            if (m_inputShape[3] > 0) {
                m_inputWidth = static_cast<int>(m_inputShape[3]);
            }
        }

        return true;
    } catch (const std::exception &) {
        // reset(): unique_ptr成员函数，调用析构函数释放内存
        m_session.reset();
        m_inputNames.clear();
        m_outputNames.clear();
        return false;
    }
}

bool yoloSeg::isLoaded() const
{
    return static_cast<bool>(m_session);
}

cv::Mat yoloSeg::preprocess(const cv::Mat &color, LetterboxInfo &info) const
{
    cv::Mat bgr;
    if (color.channels() == 4) {
        cv::cvtColor(color, bgr, cv::COLOR_BGRA2BGR);
    } else if (color.channels() == 1) {
        cv::cvtColor(color, bgr, cv::COLOR_GRAY2BGR);
    } else {
        bgr = color;
    }

    const float scale = std::min(
        static_cast<float>(m_inputWidth) / static_cast<float>(bgr.cols),
        static_cast<float>(m_inputHeight) / static_cast<float>(bgr.rows));
    const int resizedWidth = std::max(1, static_cast<int>(std::round(bgr.cols * scale)));
    const int resizedHeight = std::max(1, static_cast<int>(std::round(bgr.rows * scale)));
    const int padX = (m_inputWidth - resizedWidth) / 2;
    const int padY = (m_inputHeight - resizedHeight) / 2;

    cv::Mat resized;
    cv::resize(bgr, resized, cv::Size(resizedWidth, resizedHeight));

    cv::Mat letterboxed(
        m_inputHeight,
        m_inputWidth,
        CV_8UC3,
        cv::Scalar(114, 114, 114));
    resized.copyTo(letterboxed(cv::Rect(padX, padY, resizedWidth, resizedHeight)));

    cv::Mat rgb;
    cv::cvtColor(letterboxed, rgb, cv::COLOR_BGR2RGB);
    rgb.convertTo(rgb, CV_32FC3, 1.0 / 255.0);

    info.scale = scale;
    info.padX = padX;
    info.padY = padY;
    info.resizedWidth = resizedWidth;
    info.resizedHeight = resizedHeight;

    std::vector<cv::Mat> channels(3);
    cv::split(rgb, channels);

    cv::Mat blob(1, 3 * m_inputHeight * m_inputWidth, CV_32F);
    float *blobData = blob.ptr<float>();
    const size_t planeSize = static_cast<size_t>(m_inputHeight * m_inputWidth);
    for (int c = 0; c < 3; ++c) {
        std::memcpy(
            blobData + c * planeSize,
            channels[c].ptr<float>(),
            planeSize * sizeof(float));
    }

    return blob;
}

cv::Mat yoloSeg::postprocess(
    const std::vector<Ort::Value> &outputs,
    const LetterboxInfo &info,
    const cv::Size &originalSize) const
{
    if (outputs.size() < 2) {
        return {};
    }

    auto detShape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    auto protoShape = outputs[1].GetTensorTypeAndShapeInfo().GetShape();
    if (detShape.size() != 3 || protoShape.size() != 4) {
        return {};
    }

    const float *detData = outputs[0].GetTensorData<float>();
    const float *protoData = outputs[1].GetTensorData<float>();

    const bool transposed = detShape[1] < detShape[2];
    const int attrCount = static_cast<int>(transposed ? detShape[1] : detShape[2]);
    const int predictionCount = static_cast<int>(transposed ? detShape[2] : detShape[1]);
    const int maskCount = static_cast<int>(protoShape[1]);
    const int maskHeight = static_cast<int>(protoShape[2]);
    const int maskWidth = static_cast<int>(protoShape[3]);
    const bool endToEndOutput = attrCount == maskCount + 6;
    const int classCountNoObjectness = attrCount - 4 - maskCount;
    const int classCountWithObjectness = attrCount - 5 - maskCount;
    const bool hasObjectness = classCountWithObjectness > 0
        && classCountNoObjectness != 80
        && classCountWithObjectness <= classCountNoObjectness;
    const int classOffset = hasObjectness ? 5 : 4;
    const int classCount = hasObjectness ? classCountWithObjectness : classCountNoObjectness;
    if ((!endToEndOutput && classCount <= kPersonClassId) || maskCount <= 0) {
        return {};
    }

    //【&｜=｜null】 捕获，获取外部变量。推荐显示指定，获取正确参数，同时避免大对象的拷贝
    // mutable可以真实修改变量的数值，否则一般只是值拷贝，无法修改
    // noexcept 表示绝对不会出现异常。对于vector扩容的搬移策略有重要的性能影响
    auto at = [&](int predictionIndex, int attrIndex) {
        if (transposed) {
            return detData[attrIndex * predictionCount + predictionIndex];
        }
        return detData[predictionIndex * attrCount + attrIndex];
    };

    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    std::vector<std::vector<float>> maskCoefficients;
    for (int i = 0; i < predictionCount; ++i) {
        float personScore = 0.0f;
        float x1 = 0.0f;
        float y1 = 0.0f;
        float x2 = 0.0f;
        float y2 = 0.0f;
        int maskOffset = attrCount - maskCount;

        if (endToEndOutput) {
            const int classId = static_cast<int>(std::round(at(i, 5)));
            if (classId != kPersonClassId) {
                continue;
            }

            personScore = at(i, 4);
            x1 = (at(i, 0) - info.padX) / info.scale;
            y1 = (at(i, 1) - info.padY) / info.scale;
            x2 = (at(i, 2) - info.padX) / info.scale;
            y2 = (at(i, 3) - info.padY) / info.scale;
            maskOffset = 6;
        } else {
            const float objectness = hasObjectness ? at(i, 4) : 1.0f;
            personScore = objectness * at(i, classOffset + kPersonClassId);

            const float cx = at(i, 0);
            const float cy = at(i, 1);
            const float width = at(i, 2);
            const float height = at(i, 3);

            x1 = (cx - width * 0.5f - info.padX) / info.scale;
            y1 = (cy - height * 0.5f - info.padY) / info.scale;
            x2 = (cx + width * 0.5f - info.padX) / info.scale;
            y2 = (cy + height * 0.5f - info.padY) / info.scale;
        }

        if (personScore < confidenceThreshold()) {
            continue;
        }

        const int left = static_cast<int>(std::round(clampFloat(x1, 0.0f, static_cast<float>(originalSize.width - 1))));
        const int top = static_cast<int>(std::round(clampFloat(y1, 0.0f, static_cast<float>(originalSize.height - 1))));
        const int right = static_cast<int>(std::round(clampFloat(x2, 0.0f, static_cast<float>(originalSize.width - 1))));
        const int bottom = static_cast<int>(std::round(clampFloat(y2, 0.0f, static_cast<float>(originalSize.height - 1))));
        if (right <= left || bottom <= top) {
            continue;
        }

        boxes.emplace_back(left, top, right - left, bottom - top);
        scores.push_back(personScore);

        std::vector<float> coefficients(maskCount);
        for (int m = 0; m < maskCount; ++m) {
            coefficients[m] = at(i, maskOffset + m);
        }
        maskCoefficients.push_back(std::move(coefficients));
    }

    std::vector<int> keptIndices;
    cv::dnn::NMSBoxes(
        boxes,
        scores,
        confidenceThreshold(),
        nmsThreshold(),
        keptIndices);
    if (keptIndices.empty()) {
        return cv::Mat::zeros(originalSize, CV_8UC1);
    }

    cv::Mat personMask = cv::Mat::zeros(originalSize, CV_8UC1);
    const cv::Rect letterboxRoi(
        info.padX,
        info.padY,
        info.resizedWidth,
        info.resizedHeight);

    for (int keptIndex : keptIndices) {
        const auto &coefficients = maskCoefficients[keptIndex];
        cv::Mat maskLogits(maskHeight, maskWidth, CV_32F, cv::Scalar(0));

        for (int m = 0; m < maskCount; ++m) {
            const float coefficient = coefficients[m];
            const float *protoChannel = protoData + static_cast<size_t>(m) * maskHeight * maskWidth;
            for (int y = 0; y < maskHeight; ++y) {
                float *row = maskLogits.ptr<float>(y);
                const float *protoRow = protoChannel + static_cast<size_t>(y) * maskWidth;
                for (int x = 0; x < maskWidth; ++x) {
                    row[x] += coefficient * protoRow[x];
                }
            }
        }

        for (int y = 0; y < maskHeight; ++y) {
            float *row = maskLogits.ptr<float>(y);
            for (int x = 0; x < maskWidth; ++x) {
                row[x] = sigmoid(row[x]);
            }
        }

        cv::Mat inputMask;
        cv::resize(maskLogits, inputMask, cv::Size(m_inputWidth, m_inputHeight), 0, 0, cv::INTER_LINEAR);
        cv::Mat unpadded = inputMask(letterboxRoi);
        cv::Mat originalMask;
        cv::resize(unpadded, originalMask, originalSize, 0, 0, cv::INTER_LINEAR);

        cv::Mat binaryMask;
        cv::threshold(originalMask, binaryMask, maskThreshold(), 255.0, cv::THRESH_BINARY);
        binaryMask.convertTo(binaryMask, CV_8UC1);

        cv::Mat boxLimited = cv::Mat::zeros(originalSize, CV_8UC1);
        binaryMask(boxes[keptIndex]).copyTo(boxLimited(boxes[keptIndex]));
        cv::bitwise_or(personMask, boxLimited, personMask);
    }

    cv::morphologyEx(
        personMask,
        personMask,
        cv::MORPH_CLOSE,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

    return personMask;
}

cv::Mat yoloSeg::Segmatation(const cv::Mat &color)
{
    if (color.empty() || !m_session) {
        return {};
    }

    LetterboxInfo info;
    cv::Mat blob = preprocess(color, info);

    std::vector<int64_t> inputShape = m_inputShape;
    if (inputShape.size() != 4) {
        inputShape = {1, 3, m_inputHeight, m_inputWidth};
    } else {
        inputShape[0] = 1;
        inputShape[1] = 3;
        inputShape[2] = m_inputHeight;
        inputShape[3] = m_inputWidth;
    }

    std::vector<const char *> inputNames;
    std::vector<const char *> outputNames;
    inputNames.reserve(m_inputNames.size());
    outputNames.reserve(m_outputNames.size());
    for (const auto &name : m_inputNames) {
        inputNames.push_back(name.c_str());
    }
    for (const auto &name : m_outputNames) {
        outputNames.push_back(name.c_str());
    }

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator,
        OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        blob.ptr<float>(),
        static_cast<size_t>(blob.total()),
        inputShape.data(),
        inputShape.size());

    try {
        auto outputs = m_session->Run(
            Ort::RunOptions{nullptr},
            inputNames.data(),
            &inputTensor,
            1,
            outputNames.data(),
            outputNames.size());

        return postprocess(outputs, info, color.size());
    } catch (const std::exception &) {
        return {};
    }
}

float yoloSeg::confidenceThreshold() const
{
    return 0.25f;
}

float yoloSeg::maskThreshold() const
{
    return 0.5f;
}

float yoloSeg::nmsThreshold() const
{
    return 0.45f;
}
