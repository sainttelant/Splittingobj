#include <iostream>
#include <memory>
#include <chrono>
#include <time.h>
#include "detector.h"
#include "cxxopts.hpp"


std::vector<std::string> LoadNames(const std::string& path) {
    // load class names
    std::vector<std::string> class_names;
    std::ifstream infile(path);
    if (infile.is_open()) {
        std::string line;
        while (getline (infile,line)) {
            class_names.emplace_back(line);
        }
        infile.close();
    }
    else {
        std::cerr << "Error loading the class names!\n";
    }

    return class_names;
}


void Demo(cv::Mat& img,
        const std::vector<std::tuple<cv::Rect, float, int>>& data_vec,
        const std::vector<std::string>& class_names,
        bool label = true) {
    for (const auto & data : data_vec) {
        cv::Rect box;
        float score;
        int class_idx;
        std::tie(box, score, class_idx) = data;

        cv::rectangle(img, box, cv::Scalar(0, 0, 255), 2);

        if (label) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << score;
            std::string s = class_names[class_idx] + " " + ss.str();

            auto font_face = cv::FONT_HERSHEY_DUPLEX;
            auto font_scale = 1.0;
            int thickness = 1;
            int baseline=0;
            auto s_size = cv::getTextSize(s, font_face, font_scale, thickness, &baseline);
            cv::rectangle(img,
                    cv::Point(box.tl().x, box.tl().y - s_size.height - 5),
                    cv::Point(box.tl().x + s_size.width, box.tl().y),
                    cv::Scalar(0, 0, 255), -1);
            cv::putText(img, s, cv::Point(box.tl().x, box.tl().y - 5),
                        font_face , font_scale, cv::Scalar(255, 255, 255), thickness);
        }
    }

    cv::namedWindow("Result", cv::WINDOW_AUTOSIZE);
    cv::imshow("Result", img);
    cv::waitKey(0);
}


int main_01() {
   // cxxopts::Options parser(argv[0], "A LibTorch inference implementation of the yolov5");

    // TODO: add other args
   /* parser.allow_unrecognised_options().add_options()
            ("weights", "model.torchscript.pt path", cxxopts::value<std::string>())
            ("source", "source", cxxopts::value<std::string>())
            ("conf-thres", "object confidence threshold", cxxopts::value<float>()->default_value("0.4"))
            ("iou-thres", "IOU threshold for NMS", cxxopts::value<float>()->default_value("0.5"))
            ("gpu", "Enable cuda device or cpu", cxxopts::value<bool>()->default_value("false"))
            ("view-img", "display results", cxxopts::value<bool>()->default_value("false"))
            ("h,help", "Print usage");

    auto opt = parser.parse(argc, argv);*/

    //if (opt.count("help")) {
    //    std::cout << parser.help() << std::endl;
    //    exit(0);
    //}

    // check if gpu flag is set
    bool is_gpu = false;

    // set device type - CPU/GPU

    torch::DeviceType device_type;
    if (torch::cuda::is_available() && is_gpu) {
        device_type = torch::kCUDA;
    } else {
        device_type = torch::kCPU;
    }

    // load class names from dataset for visualization
    std::vector<std::string> class_names = LoadNames("../../weights/coco.names");
    if (class_names.empty()) {
        std::cout << "load className failed!" << std::endl;   
        return -1;
    }
    else
    {
        std::cout << "load className success!" << std::endl;
    }

    // load input image
    // std::string source = opt["source"].as<std::string>();
    std::string source = "../../images/save.jpg";
    cv::Mat img = cv::imread(source);
    if (img.empty()) {
        std::cerr << "Error loading the image!\n";
        return -1;
    }

    // load network
    std::string weights = "../../weights/yolov5s.torchscript.pt";
    auto detector = Detector(weights, device_type);

    // inference
    float conf_thres = 0.5f;
    float iou_thres = 0.5f;
    auto result = detector.Run(img, conf_thres, iou_thres);

  //  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // It should be known that it takes longer time at first time
   // std::cout << "inference taken : " << duration.count() << " ms" << std::endl;

    // visualize detections
    if (1) {
        Demo(img, result, class_names);
    }
    system("pause");
    return 0;
}