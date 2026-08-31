#include "faciallandmark.h"

#include <cstdlib>
#include <array>
#include <filesystem>
#include <iostream>
#include <string>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

namespace {

struct Options {
    std::string modelPath;
    std::string imagePath;
    std::string savePath;
    int cameraIndex = 0;
    bool useCamera = true;
};

std::string defaultModelPath()
{
#ifdef REALSENSE_SNAP_RESOURCE_DIR
    return (std::filesystem::path(REALSENSE_SNAP_RESOURCE_DIR) / "models" / "yolov8n-face.onnx").string();
#else
    return "resources/models/yolov8n-face.onnx";
#endif
}

void printUsage(const char *programName)
{
    std::cout
        << "Usage:\n"
        << "  " << programName << " [--camera index] [--model path]\n"
        << "  " << programName << " --image path [--save path] [--model path]\n"
        << "\n"
        << "Options:\n"
        << "  --camera index  Open camera by index. Default: 0\n"
        << "  --image path    Run detection on one image instead of camera.\n"
        << "  --model path    YOLOv8 face ONNX model path.\n"
        << "  --save path     Save rendered image result. Only used with --image.\n"
        << "  --help          Show this message.\n";
}

bool parseCameraIndex(const std::string &value, int &cameraIndex)
{
    try {
        size_t consumed = 0;
        cameraIndex = std::stoi(value, &consumed);
        return consumed == value.size() && cameraIndex >= 0;
    } catch (const std::exception &) {
        return false;
    }
}

bool parseOptions(int argc, char *argv[], Options &options)
{
    options.modelPath = defaultModelPath();

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            std::exit(EXIT_SUCCESS);
        }

        if (arg == "--model" || arg == "-m") {
            if (++i >= argc) {
                std::cerr << "ERROR: --model requires a path.\n";
                return false;
            }
            options.modelPath = argv[i];
        } else if (arg == "--image" || arg == "-i") {
            if (++i >= argc) {
                std::cerr << "ERROR: --image requires a path.\n";
                return false;
            }
            options.imagePath = argv[i];
            options.useCamera = false;
        } else if (arg == "--camera" || arg == "-c") {
            if (++i >= argc) {
                std::cerr << "ERROR: --camera requires an index.\n";
                return false;
            }
            if (!parseCameraIndex(argv[i], options.cameraIndex)) {
                std::cerr << "ERROR: Invalid camera index: " << argv[i] << "\n";
                return false;
            }
            options.useCamera = true;
        } else if (arg == "--save" || arg == "-s") {
            if (++i >= argc) {
                std::cerr << "ERROR: --save requires a path.\n";
                return false;
            }
            options.savePath = argv[i];
        } else if (options.imagePath.empty()) {
            options.imagePath = arg;
            options.useCamera = false;
        } else {
            std::cerr << "ERROR: Unknown argument: " << arg << "\n";
            return false;
        }
    }

    return true;
}

cv::Mat drawDetections(const cv::Mat &image, const std::vector<facialLandmark::Face> &faces)
{
    cv::Mat result = image.clone();
    static const std::array<cv::Scalar, 5> landmarkColors = {
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 255, 255),
        cv::Scalar(255, 0, 0),
        cv::Scalar(255, 0, 255),
        cv::Scalar(0, 128, 255),
    };

    for (const auto &face : faces) {
        cv::rectangle(result, face.rect, cv::Scalar(0, 0, 255), 2);
        cv::putText(
            result,
            cv::format("%.2f", face.confidence),
            cv::Point(face.rect.x, std::max(face.rect.y - 6, 14)),
            cv::FONT_HERSHEY_SIMPLEX,
            0.5,
            cv::Scalar(0, 255, 0),
            1,
            cv::LINE_AA);

        for (size_t i = 0; i < face.landmarks.size(); ++i) {
            cv::circle(result, face.landmarks[i], 2, landmarkColors[i % landmarkColors.size()], -1, cv::LINE_AA);
        }
    }

    return result;
}

int runImageDemo(facialLandmark &detector, const Options &options)
{
    cv::Mat image = cv::imread(options.imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "ERROR: Cannot read image: " << options.imagePath << "\n";
        return EXIT_FAILURE;
    }

    const auto faces = detector.detect(image);
    cv::Mat result = drawDetections(image, faces);

    std::cout << "Detected faces: " << faces.size() << "\n";
    for (size_t i = 0; i < faces.size(); ++i) {
        const auto &face = faces[i];
        std::cout
            << "  #" << i + 1
            << " score=" << face.confidence
            << " rect=(" << face.rect.x << "," << face.rect.y
            << "," << face.rect.width << "," << face.rect.height << ")"
            << " landmarks=" << face.landmarks.size()
            << "\n";
    }

    if (!options.savePath.empty()) {
        if (!cv::imwrite(options.savePath, result)) {
            std::cerr << "ERROR: Cannot save result to: " << options.savePath << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Saved result: " << options.savePath << "\n";
    }

    cv::imshow("YOLOv8 Face Landmark Demo", result);
    cv::waitKey(0);
    cv::destroyAllWindows();
    return EXIT_SUCCESS;
}

int runCameraDemo(facialLandmark &detector, const Options &options)
{
    cv::VideoCapture capture(options.cameraIndex);
    if (!capture.isOpened()) {
        std::cerr << "ERROR: Cannot open camera index: " << options.cameraIndex << "\n";
        return EXIT_FAILURE;
    }

    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    std::cout << "Press ESC or q to exit.\n";
    cv::Mat frame;
    while (true) {
        capture >> frame;
        if (frame.empty()) {
            std::cerr << "WARNING: Empty camera frame.\n";
            break;
        }

        cv::Mat result = detector.LandmarkDetection(frame.clone());
        cv::imshow("YOLOv8 Face Landmark Demo", result);

        const char key = static_cast<char>(cv::waitKey(1));
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }

    cv::destroyAllWindows();
    return EXIT_SUCCESS;
}

}

int main(int argc, char *argv[])
{
    Options options;
    if (!parseOptions(argc, argv, options)) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!std::filesystem::exists(options.modelPath)) {
        std::cerr << "ERROR: Model file does not exist: " << options.modelPath << "\n";
        return EXIT_FAILURE;
    }

    facialLandmark detector(options.modelPath);
    if (!detector.isLoaded()) {
        std::cerr << "ERROR: Failed to load model: " << options.modelPath << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Model: " << options.modelPath << "\n";
    if (options.useCamera) {
        return runCameraDemo(detector, options);
    }

    return runImageDemo(detector, options);
}
