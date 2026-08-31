#include "faciallandmark.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

namespace {
constexpr int kClassCount = 1;
constexpr int kRegMax = 16;
constexpr int kLandmarkCount = 5;
constexpr int kValuesPerLandmark = 3;
constexpr int kBoxChannelCount = kRegMax * 4;
constexpr int kExpectedChannelCount = kBoxChannelCount + kClassCount + kLandmarkCount * kValuesPerLandmark;

float clampFloat(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}
}

facialLandmark::facialLandmark()
{
    if (!loadModel(defaultModelPath())) {
        std::cerr << "Warning: failed to load YOLOv8 face model." << std::endl;
    }
}

facialLandmark::facialLandmark(const std::string &modelPath)
{
    if (!loadModel(modelPath)) {
        std::cerr << "Warning: failed to load YOLOv8 face model: " << modelPath << std::endl;
    }
}

facialLandmark::~facialLandmark() = default;

bool facialLandmark::loadModel(const std::string &modelPath)
{
    try {
        m_net = cv::dnn::readNet(modelPath);
        if (m_net.empty()) {
            m_loaded = false;
            return false;
        }

        m_modelPath = modelPath;
        m_loaded = true;
        return true;
    } catch (const cv::Exception &e) {
        std::cerr << "Load YOLOv8 face model error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Load YOLOv8 face model error: " << e.what() << std::endl;
    }

    m_net = cv::dnn::Net();
    m_modelPath.clear();
    m_loaded = false;
    return false;
}

bool facialLandmark::loadLandmarkModel(const std::string &modelPath)
{
    return loadModel(modelPath);
}

bool facialLandmark::loadHearModel(const std::string &modelPath)
{
    (void)modelPath;
    return true;
}

bool facialLandmark::isLoaded() const
{
    return m_loaded;
}

std::vector<facialLandmark::Face> facialLandmark::detect(const cv::Mat &color)
{
    std::vector<Face> faces;
    if (color.empty() || !m_loaded) {
        return faces;
    }

    cv::Mat bgr;
    if (color.channels() == 4) {
        cv::cvtColor(color, bgr, cv::COLOR_BGRA2BGR);
    } else if (color.channels() == 1) {
        cv::cvtColor(color, bgr, cv::COLOR_GRAY2BGR);
    } else {
        bgr = color;
    }

    LetterboxInfo info;
    cv::Mat input = resizeImage(bgr, info);

    cv::Mat blob;
    cv::dnn::blobFromImage(
        input,
        blob,
        1.0 / 255.0,
        cv::Size(m_inputWidth, m_inputHeight),
        cv::Scalar(0, 0, 0),
        true,
        false);

    try {
        m_net.setInput(blob);
        m_net.enableWinograd(false);

        std::vector<cv::Mat> outputs;
        m_net.forward(outputs, m_net.getUnconnectedOutLayersNames());

        std::vector<cv::Rect> boxes;
        std::vector<float> confidences;
        std::vector<std::vector<cv::Point>> landmarks;
        for (const cv::Mat &output : outputs) {
            generateProposal(output, boxes, confidences, landmarks, bgr.size(), info);
        }

        std::vector<int> keptIndices;
        cv::dnn::NMSBoxes(
            boxes,
            confidences,
            m_confidenceThreshold,
            m_nmsThreshold,
            keptIndices);

        faces.reserve(keptIndices.size());
        for (int index : keptIndices) {
            faces.push_back({boxes[index], confidences[index], landmarks[index]});
        }
    } catch (const cv::Exception &e) {
        std::cerr << "YOLOv8 face inference error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "YOLOv8 face inference error: " << e.what() << std::endl;
    }

    return faces;
}

cv::Mat facialLandmark::LandmarkDetection(cv::Mat color)
{
    if (color.empty()) {
        return color;
    }

    cv::Mat draw = color;
    const std::vector<Face> faces = detect(color);
    static const std::array<cv::Scalar, kLandmarkCount> landmarkColors = {
        cv::Scalar(0, 255, 0, 255),
        cv::Scalar(0, 255, 255, 255),
        cv::Scalar(255, 0, 0, 255),
        cv::Scalar(255, 0, 255, 255),
        cv::Scalar(0, 128, 255, 255),
    };

    for (const Face &face : faces) {
        cv::rectangle(draw, face.rect, cv::Scalar(0, 0, 255, 255), 2);
        const std::string label = cv::format("%.2f", face.confidence);
        const int labelY = std::max(face.rect.y - 6, 14);
        cv::putText(
            draw,
            label,
            cv::Point(face.rect.x, labelY),
            cv::FONT_HERSHEY_SIMPLEX,
            0.5,
            cv::Scalar(0, 255, 0, 255),
            1,
            cv::LINE_AA);

        for (size_t i = 0; i < face.landmarks.size(); ++i) {
            cv::circle(draw, face.landmarks[i], 2, landmarkColors[i % landmarkColors.size()], -1, cv::LINE_AA);
        }
    }

    return draw;
}

cv::Mat facialLandmark::resizeImage(const cv::Mat &src, LetterboxInfo &info) const
{
    const int srcHeight = src.rows;
    const int srcWidth = src.cols;
    float scale = std::min(
        static_cast<float>(m_inputWidth) / static_cast<float>(srcWidth),
        static_cast<float>(m_inputHeight) / static_cast<float>(srcHeight));

    info.resizedWidth = std::max(1, static_cast<int>(std::round(srcWidth * scale)));
    info.resizedHeight = std::max(1, static_cast<int>(std::round(srcHeight * scale)));
    info.padX = (m_inputWidth - info.resizedWidth) / 2;
    info.padY = (m_inputHeight - info.resizedHeight) / 2;
    info.ratioX = static_cast<float>(srcWidth) / static_cast<float>(info.resizedWidth);
    info.ratioY = static_cast<float>(srcHeight) / static_cast<float>(info.resizedHeight);

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(info.resizedWidth, info.resizedHeight), 0, 0, cv::INTER_AREA);

    cv::Mat letterboxed(m_inputHeight, m_inputWidth, src.type(), cv::Scalar(0, 0, 0));
    resized.copyTo(letterboxed(cv::Rect(info.padX, info.padY, info.resizedWidth, info.resizedHeight)));
    return letterboxed;
}

void facialLandmark::generateProposal(
    const cv::Mat &output,
    std::vector<cv::Rect> &boxes,
    std::vector<float> &confidences,
    std::vector<std::vector<cv::Point>> &landmarks,
    const cv::Size &originalSize,
    const LetterboxInfo &info) const
{
    if (output.dims != 4 || output.size[1] < kExpectedChannelCount) {
        return;
    }

    const int featureHeight = output.size[2];
    const int featureWidth = output.size[3];
    const int stride = static_cast<int>(std::ceil(static_cast<float>(m_inputHeight) / featureHeight));
    const int area = featureHeight * featureWidth;
    const float *data = reinterpret_cast<const float *>(output.data);
    const float *classData = data + area * kBoxChannelCount;
    const float *landmarkData = data + area * (kBoxChannelCount + kClassCount);

    std::array<float, kRegMax> dflValues{};
    std::array<float, kRegMax> dflSoftmax{};
    for (int y = 0; y < featureHeight; ++y) {
        for (int x = 0; x < featureWidth; ++x) {
            const int index = y * featureWidth + x;
            const float confidence = sigmoid(classData[index]);
            if (confidence < m_confidenceThreshold) {
                continue;
            }

            std::array<float, 4> distances{};
            for (int side = 0; side < 4; ++side) {
                for (int bin = 0; bin < kRegMax; ++bin) {
                    dflValues[bin] = data[(side * kRegMax + bin) * area + index];
                }

                softmax(dflValues.data(), dflSoftmax.data(), kRegMax);
                float distance = 0.0f;
                for (int bin = 0; bin < kRegMax; ++bin) {
                    distance += static_cast<float>(bin) * dflSoftmax[bin];
                }
                distances[side] = distance * stride;
            }

            const float centerX = (static_cast<float>(x) + 0.5f) * stride;
            const float centerY = (static_cast<float>(y) + 0.5f) * stride;
            const float x1 = clampFloat((centerX - distances[0] - info.padX) * info.ratioX, 0.0f, static_cast<float>(originalSize.width - 1));
            const float y1 = clampFloat((centerY - distances[1] - info.padY) * info.ratioY, 0.0f, static_cast<float>(originalSize.height - 1));
            const float x2 = clampFloat((centerX + distances[2] - info.padX) * info.ratioX, 0.0f, static_cast<float>(originalSize.width - 1));
            const float y2 = clampFloat((centerY + distances[3] - info.padY) * info.ratioY, 0.0f, static_cast<float>(originalSize.height - 1));
            if (x2 <= x1 || y2 <= y1) {
                continue;
            }

            boxes.emplace_back(
                static_cast<int>(std::round(x1)),
                static_cast<int>(std::round(y1)),
                static_cast<int>(std::round(x2 - x1)),
                static_cast<int>(std::round(y2 - y1)));
            confidences.push_back(confidence);

            std::vector<cv::Point> points;
            points.reserve(kLandmarkCount);
            for (int pointIndex = 0; pointIndex < kLandmarkCount; ++pointIndex) {
                const float rawX = landmarkData[(pointIndex * kValuesPerLandmark) * area + index];
                const float rawY = landmarkData[(pointIndex * kValuesPerLandmark + 1) * area + index];
                const float mappedX = ((rawX * 2.0f + x) * stride - info.padX) * info.ratioX;
                const float mappedY = ((rawY * 2.0f + y) * stride - info.padY) * info.ratioY;
                points.emplace_back(
                    static_cast<int>(std::round(clampFloat(mappedX, 0.0f, static_cast<float>(originalSize.width - 1)))),
                    static_cast<int>(std::round(clampFloat(mappedY, 0.0f, static_cast<float>(originalSize.height - 1)))));
            }
            landmarks.push_back(std::move(points));
        }
    }
}

void facialLandmark::softmax(const float *src, float *dst, int length)
{
    float maxValue = src[0];
    for (int i = 1; i < length; ++i) {
        maxValue = std::max(maxValue, src[i]);
    }

    float sum = 0.0f;
    for (int i = 0; i < length; ++i) {
        dst[i] = std::exp(src[i] - maxValue);
        sum += dst[i];
    }

    if (sum <= 0.0f) {
        return;
    }

    for (int i = 0; i < length; ++i) {
        dst[i] /= sum;
    }
}

float facialLandmark::sigmoid(float value)
{
    return 1.0f / (1.0f + std::exp(-value));
}

std::string facialLandmark::defaultModelPath()
{
#ifdef REALSENSE_SNAP_RESOURCE_DIR
    const std::filesystem::path configuredResourceDir(REALSENSE_SNAP_RESOURCE_DIR);
    const std::filesystem::path configuredModelPath = configuredResourceDir / "models" / "yolov8n-face.onnx";
    if (std::filesystem::exists(configuredModelPath)) {
        return configuredModelPath.string();
    }
#endif

    const std::array<std::filesystem::path, 5> candidates = {
        std::filesystem::path("resources/models/yolov8n-face.onnx"),
        std::filesystem::path("../resources/models/yolov8n-face.onnx"),
        std::filesystem::path("../../resources/models/yolov8n-face.onnx"),
        std::filesystem::path("../../../resources/models/yolov8n-face.onnx"),
        std::filesystem::path("../../../../resources/models/yolov8n-face.onnx"),
    };

    for (const auto &candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    return candidates.front().string();
}
