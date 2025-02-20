# yabloc_pose_initializer

This package contains some nodes related to initial pose estimation.

- [camera_pose_initializer](#camera_pose_initializer)
- [semantic_segmentation_server](#semantic_segmentation_server)

## Note

This package makes use of external code. The trained files are provided by apollo. The trained files are automatically downloaded when you build.

Original model URL

<https://github.com/openvinotoolkit/open_model_zoo/tree/master/models/intel/road-segmentation-adas-0001>

> Open Model Zoo is licensed under Apache License Version 2.0.

Converted model URL

<https://github.com/PINTO0309/PINTO_model_zoo/tree/main/136_road-segmentation-adas-0001>

> model conversion scripts are released under the MIT license

## Special thanks

- [openvinotoolkit/open_model_zoo](https://github.com/openvinotoolkit/open_model_zoo)
- [PINTO0309](https://github.com/PINTO0309)

## camera_pose_initializer

### Purpose

- This node estimates the initial position using the camera at the request of ADAPI.

#### Input

| Name                | Type                                         | Description              |
| ------------------- | -------------------------------------------- | ------------------------ |
| `input/camera_info` | `sensor_msgs::msg::CameraInfo`               | undistorted camera info  |
| `input/image_raw`   | `sensor_msgs::msg::Image`                    | undistorted camera image |
| `input/vector_map`  | `autoware_auto_mapping_msgs::msg::HADMapBin` | vector map               |

#### Output

| Name                | Type                                   | Description             |
| ------------------- | -------------------------------------- | ----------------------- |
| `output/candidates` | `visualization_msgs::msg::MarkerArray` | initial pose candidates |

### Parameters

| Name               | Type | Description                               |
| ------------------ | ---- | ----------------------------------------- |
| `angle_resolution` | int  | how many divisions of 1 sigma angle range |

### Services

| Name               | Type                                                      | Description                     |
| ------------------ | --------------------------------------------------------- | ------------------------------- |
| `yabloc_align_srv` | `tier4_localization_msgs::srv::PoseWithCovarianceStamped` | initial pose estimation request |

### Clients

| Name                        | Type                                                 | Description                   |
| --------------------------- | ---------------------------------------------------- | ----------------------------- |
| `semantic_segmentation_srv` | `yabloc_pose_initializer::srv::SemanticSegmentation` | semantic segmentation request |

## semantic_segmentation_server

### Purpose

- This node performs semantic segmentation.

### Services

| Name                        | Type                                                 | Description                   |
| --------------------------- | ---------------------------------------------------- | ----------------------------- |
| `semantic_segmentation_srv` | `yabloc_pose_initializer::srv::SemanticSegmentation` | semantic segmentation request |
