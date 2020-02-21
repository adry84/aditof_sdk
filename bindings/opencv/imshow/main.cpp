#include <aditof/camera.h>
#include <aditof/camera_96tof1_specifics.h>
#include <aditof/device_interface.h>
#include <aditof/frame.h>
#include <aditof/system.h>
#include <glog/logging.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#ifdef OPENCV2
#include <opencv2/contrib/contrib.hpp>
#endif

#include "../aditof_opencv.h"

using namespace aditof;

int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = 1;

    Status status = Status::OK;

    System system;
    status = system.initialize();
    if (status != Status::OK) {
        LOG(ERROR) << "Could not initialize system!";
        return 0;
    }

    std::vector<std::shared_ptr<Camera>> cameras;
    system.getCameraList(cameras);
    if (cameras.empty()) {
        LOG(WARNING) << "No cameras found!";
        return 0;
    }

    auto camera = cameras.front();
    status = camera->initialize();
    if (status != Status::OK) {
        LOG(ERROR) << "Could not initialize camera!";
        return 0;
    }

    std::vector<std::string> frameTypes;
    camera->getAvailableFrameTypes(frameTypes);
    if (frameTypes.empty()) {
        LOG(ERROR) << "No frame type available!";
        return 0;
    }

    status = camera->setFrameType(frameTypes.front());
    if (status != Status::OK) {
        LOG(ERROR) << "Could not set camera frame type!";
        return 0;
    }

    std::vector<std::string> modes;
    camera->getAvailableModes(modes);
    if (modes.empty()) {
        LOG(ERROR) << "No camera modes available!";
        return 0;
    }

    status = camera->setMode(modes[0]);
    if (status != Status::OK) {
        LOG(ERROR) << "Could not set camera mode!";
        return 0;
    }

    aditof::CameraDetails cameraDetails;
    camera->getDetails(cameraDetails);
    int cameraRange = cameraDetails.maxDepth;
    aditof::Frame frame;

    const int smallSignalThreshold = 50;
    auto specifics = camera->getSpecifics();
    auto cam96tof1Specifics =
        std::dynamic_pointer_cast<Camera96Tof1Specifics>(specifics);
    cam96tof1Specifics->setNoiseReductionThreshold(smallSignalThreshold);
    cam96tof1Specifics->enableNoiseReduction(true);

    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);

    while (cv::waitKey(1) != 27 &&
           getWindowProperty("Display Image", cv::WND_PROP_AUTOSIZE) >= 0) {

        /* Request frame from camera */
        status = camera->requestFrame(&frame);
        if (status != Status::OK) {
            LOG(ERROR) << "Could not request frame!";
            return 0;
        }

        /* Convert from frame to depth mat */
        cv::Mat mat;
        status = fromFrameToDepthMat(frame, mat);
        if (status != Status::OK) {
            LOG(ERROR) << "Could not convert from frame to mat!";
            return 0;
        }

        /* Distance factor */
        double distance_scale = 255.0 / cameraRange;

        /* Convert from raw values to values that opencv can understand */
        mat.convertTo(mat, CV_8U, distance_scale);

        /* Apply a rainbow color map to the mat to better visualize the
         * depth data */
        applyColorMap(mat, mat, cv::COLORMAP_RAINBOW);

        /* Display the image */
        imshow("Display Image", mat);
    }

    return 0;
}
