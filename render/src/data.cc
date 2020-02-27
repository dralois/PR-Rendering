/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include <iomanip>      // std::setfill, std::setw
#include <sstream>
#include <iostream>

#include "../include/data.h"
#include "../include/util.h"

namespace utils_funct {

std::vector<std::string> split(const std::string s, const std::string delim) {
    std::vector<std::string> list;
    auto start = 0U;
    auto end = s.find(delim);
    while (true) {
        list.push_back(s.substr(start, end - start));
        if (end == std::string::npos)
            break;
        start = end + delim.length();
        end = s.find(delim, start);
    }
    return list;
}

};

const std::string Data::getCalibFile() const {
    return dataPath + "/" + calibFile;
};

void Data::NextFrame() {
    frame_id++;
}

Data::Data(const std::string& path): dataPath(path) {
}

void Data::LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose) {
    std::ifstream file(pose_file);
    if (file.is_open()) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                file >> pose(i, j);
        file.close();
    }
}

Eigen::Matrix4f Data::LoadViewMatrix(std::string cam_pose) {
    Eigen::Matrix4f camera_pose;
    Eigen::Vector3f camera_direction;
    Eigen::Vector3f camera_right;
    Eigen::Vector3f camera_up;
    Eigen::Vector3f camera_eye;
    Eigen::Vector3f camera_center;
    Eigen::Matrix4f view_pose;

    LoadPose(cam_pose , camera_pose);
    camera_direction = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(0, 0, 1);
    camera_right = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(1, 0, 0);
    camera_up = camera_right.cross(camera_direction);
    camera_eye = camera_pose.block<3, 1>(0, 3);
    camera_center = camera_eye + 1 * camera_direction;

    view_pose = C3DV_camera::lookAt(camera_eye, camera_center, camera_up);
    return view_pose;
}

void Data::LoadViewMatrix() {
    int frame_id = 0;
    Eigen::Matrix4f camera_pose;
    Eigen::Vector3f camera_direction;
    Eigen::Vector3f camera_right;
    Eigen::Vector3f camera_up;
    Eigen::Vector3f camera_eye;
    Eigen::Vector3f camera_center;
    Eigen::Matrix4f view_pose;

    while (true) {
        std::stringstream filename;
        filename << dataPath << pose_prefix_ << std::setfill('0') << std::setw(6) << int(frame_id) << pose_suffix_;
        if (!FileExists(filename.str()))
           break;
        LoadPose(filename.str() , camera_pose);
        frame_id++;
        camera_direction = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(0, 0, 1);
        camera_right = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(1, 0, 0);
        camera_up = camera_right.cross(camera_direction);
        camera_eye = camera_pose.block<3, 1>(0, 3);
        camera_center = camera_eye + 1 * camera_direction;

        view_pose = C3DV_camera::lookAt(camera_eye, camera_center, camera_up);
        poses_.push_back(view_pose);
    }
}

const Eigen::Matrix4f& Data::GetPose() const {
    return poses_[fmin(static_cast<int>(frame_id), poses_.size() -1)];
}

bool Data::LoadIntrinsics() {
    std::cout << dataPath << calibFile << std::endl;
    std::string line{""};
    std::cout << getCalibFile() << std::endl;
    std::ifstream file(getCalibFile());    
    if (file.is_open()) {
        while (std::getline(file,line)) {
            if (line.rfind("m_colorWidth", 0) == 0)
                intrinsics.image_width = std::stoi(line.substr(line.find("= ")+2, std::string::npos));
            else if (line.rfind("m_colorHeight", 0) == 0)
                intrinsics.image_height = std::stoi(line.substr(line.find("= ")+2, std::string::npos));
            else if (line.rfind("m_calibrationColorIntrinsic", 0) == 0) {
                const std::string model = line.substr(line.find("= ")+2, std::string::npos);
                const auto parts = utils_funct::split(model, " ");
                intrinsics.f_x = std::stof(parts[0]);
                intrinsics.f_y = std::stof(parts[5]);
                intrinsics.c_x = std::stof(parts[2]);
                intrinsics.c_y = std::stof(parts[6]);
            }
        }
        file.close();
        return true;
    }
    return false;
}


