// Copyright 2022 TIER IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "behavior_path_planner/marker_util/debug_utilities.hpp"
#include "behavior_path_planner/utils/path_shifter/path_shifter.hpp"

#include <behavior_path_planner/marker_util/lane_change/debug.hpp>
#include <tier4_autoware_utils/ros/marker_helper.hpp>

#include <autoware_auto_perception_msgs/msg/predicted_objects.hpp>
#include <autoware_auto_planning_msgs/msg/path_with_lane_id.hpp>
#include <visualization_msgs/msg/detail/marker__struct.hpp>

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace marker_utils::lane_change_markers
{
using geometry_msgs::msg::Point;
using tier4_autoware_utils::createDefaultMarker;
using tier4_autoware_utils::createMarkerColor;
using tier4_autoware_utils::createMarkerScale;

MarkerArray showObjectInfo(
  const std::unordered_map<std::string, CollisionCheckDebug> & obj_debug_vec, std::string && ns)
{
  Marker obj_marker = createDefaultMarker(
    "map", rclcpp::Clock{RCL_ROS_TIME}.now(), ns, 0L, Marker::TEXT_VIEW_FACING,
    createMarkerScale(0.5, 0.5, 0.5), createMarkerColor(0.0, 1.0, 1.0, 0.999));

  MarkerArray marker_array;
  int32_t id{0};

  marker_array.markers.reserve(obj_debug_vec.size());

  int idx{0};

  for (const auto & [uuid, info] : obj_debug_vec) {
    obj_marker.id = ++id;
    obj_marker.pose = info.current_pose;

    std::ostringstream ss;

    ss << "Idx: " << ++idx << "\nReason: " << info.failed_reason
       << "\nRSS dist: " << std::setprecision(4) << info.rss_longitudinal
       << "\nEgo to obj: " << info.ego_to_obj_margin << "\nLateral offset: " << info.lateral_offset
       << "\nLongitudinal offset: " << info.longitudinal_offset
       << "\nPosition: " << (info.is_front ? "front" : "back")
       << "\nSafe: " << (info.allow_lane_change ? "Yes" : "No");

    obj_marker.text = ss.str();

    marker_array.markers.push_back(obj_marker);
  }
  return marker_array;
}

MarkerArray showAllValidLaneChangePath(const std::vector<LaneChangePath> & lanes, std::string && ns)
{
  if (lanes.empty()) {
    return MarkerArray{};
  }

  MarkerArray marker_array;
  int32_t id{0};
  const auto current_time{rclcpp::Clock{RCL_ROS_TIME}.now()};

  constexpr auto colors = colorsList();
  const auto loop_size = std::min(lanes.size(), colors.size());
  marker_array.markers.reserve(loop_size);

  for (std::size_t idx = 0; idx < loop_size; ++idx) {
    if (lanes.at(idx).path.points.empty()) {
      continue;
    }

    const auto & color = colors.at(idx);
    Marker marker = createDefaultMarker(
      "map", current_time, ns, ++id, Marker::LINE_STRIP, createMarkerScale(0.1, 0.1, 0.0),
      createMarkerColor(color[0], color[1], color[2], 0.9));
    marker.points.reserve(lanes.at(idx).path.points.size());

    for (const auto & point : lanes.at(idx).path.points) {
      marker.points.push_back(point.point.pose.position);
    }

    marker_array.markers.push_back(marker);
  }
  return marker_array;
}

MarkerArray showLerpedPose(
  const std::unordered_map<std::string, CollisionCheckDebug> & obj_debug_vec, std::string && ns)
{
  MarkerArray marker_array;
  int32_t id{0};
  const auto current_time{rclcpp::Clock{RCL_ROS_TIME}.now()};
  marker_array.markers.reserve(obj_debug_vec.size());

  for (const auto & [uuid, info] : obj_debug_vec) {
    Marker marker = createDefaultMarker(
      "map", current_time, ns, ++id, Marker::POINTS, createMarkerScale(0.3, 0.3, 0.3),
      createMarkerColor(1.0, 0.3, 1.0, 0.9));
    marker.points.reserve(info.lerped_path.size());

    for (const auto & point : info.lerped_path) {
      marker.points.push_back(point.position);
    }

    marker_array.markers.push_back(marker);
  }
  return marker_array;
}

MarkerArray showEgoPredictedPaths(
  const std::unordered_map<std::string, CollisionCheckDebug> & obj_debug_vec, std::string && ns)
{
  if (obj_debug_vec.empty()) {
    return MarkerArray{};
  }

  MarkerArray marker_array;
  constexpr auto colors = colorsList();
  constexpr float scale_val = 0.2;
  const auto current_time{rclcpp::Clock{RCL_ROS_TIME}.now()};
  marker_array.markers.reserve(obj_debug_vec.size());

  int32_t id{0};
  for (const auto & [uuid, info] : obj_debug_vec) {
    const auto loop_size = std::min(info.ego_predicted_path.size(), colors.size());

    for (std::size_t idx = 0; idx < loop_size; ++idx) {
      const auto & path = info.ego_predicted_path.at(idx).path;
      const auto & color = colors.at(idx);

      Marker marker = createDefaultMarker(
        "map", current_time, ns, ++id, Marker::LINE_STRIP,
        createMarkerScale(scale_val, scale_val, scale_val),
        createMarkerColor(color[0], color[1], color[2], 0.9));

      marker.points.reserve(path.size());

      for (const auto & point : path) {
        marker.points.push_back(point.position);
      }

      marker_array.markers.push_back(marker);
    }
  }
  return marker_array;
}

MarkerArray showPolygon(
  const std::unordered_map<std::string, CollisionCheckDebug> & obj_debug_vec, std::string && ns)
{
  if (obj_debug_vec.empty()) {
    return MarkerArray{};
  }

  constexpr float scale_val{0.2};
  int32_t id{0};
  const auto now = rclcpp::Clock{RCL_ROS_TIME}.now();
  Marker ego_marker = createDefaultMarker(
    "map", now, ns, id, Marker::LINE_STRIP, createMarkerScale(scale_val, scale_val, scale_val),
    createMarkerColor(1.0, 1.0, 1.0, 0.9));
  Marker obj_marker = ego_marker;

  auto text_marker = createDefaultMarker(
    "map", now, ns + "_text", id, visualization_msgs::msg::Marker::TEXT_VIEW_FACING,
    createMarkerScale(1.5, 1.5, 1.5), createMarkerColor(1.0, 1.0, 1.0, 1.0));

  MarkerArray marker_array;

  constexpr auto red_color = colorsList().at(0);
  constexpr auto green_color = colorsList().at(1);
  const auto reserve_size = obj_debug_vec.size();

  marker_array.markers.reserve(reserve_size * 4);

  int32_t idx = {0};

  for (const auto & [uuid, info] : obj_debug_vec) {
    const auto & color = info.allow_lane_change ? green_color : red_color;
    const auto & ego_polygon = info.ego_polygon.outer();
    const auto poly_z = info.current_pose.position.z;  // temporally
    ego_marker.id = ++id;
    ego_marker.color = createMarkerColor(color[0], color[1], color[2], 0.8);
    ego_marker.points.reserve(ego_polygon.size());
    for (const auto & p : ego_polygon) {
      ego_marker.points.push_back(tier4_autoware_utils::createPoint(p.x(), p.y(), poly_z));
    }
    marker_array.markers.push_back(ego_marker);

    std::ostringstream ss;
    text_marker.id = ego_marker.id;
    ss << ++idx;
    text_marker.text = ss.str();
    text_marker.pose = info.expected_ego_pose;

    marker_array.markers.push_back(text_marker);

    const auto & obj_polygon = info.obj_polygon.outer();
    obj_marker.id = ++id;
    obj_marker.color = createMarkerColor(color[0], color[1], color[2], 0.8);
    obj_marker.points.reserve(obj_polygon.size());
    for (const auto & p : obj_polygon) {
      obj_marker.points.push_back(tier4_autoware_utils::createPoint(p.x(), p.y(), poly_z));
    }
    marker_array.markers.push_back(obj_marker);

    text_marker.id = obj_marker.id;
    text_marker.pose = info.expected_obj_pose;
    marker_array.markers.push_back(text_marker);

    ego_marker.points.clear();
    obj_marker.points.clear();
  }

  return marker_array;
}

MarkerArray showPolygonPose(
  const std::unordered_map<std::string, CollisionCheckDebug> & obj_debug_vec, std::string && ns)
{
  constexpr auto colors = colorsList();
  const auto loop_size = std::min(colors.size(), obj_debug_vec.size());
  MarkerArray marker_array;
  int32_t id{0};
  size_t idx{0};
  const auto current_time{rclcpp::Clock{RCL_ROS_TIME}.now()};
  marker_array.markers.reserve(obj_debug_vec.size());

  for (const auto & [uuid, info] : obj_debug_vec) {
    const auto & color = colors.at(idx);
    Marker marker = createDefaultMarker(
      "map", current_time, ns, ++id, Marker::POINTS, createMarkerScale(0.2, 0.2, 0.2),
      createMarkerColor(color[0], color[1], color[2], 0.999));
    marker.points.reserve(2);
    marker.points.push_back(info.expected_ego_pose.position);
    marker.points.push_back(info.expected_obj_pose.position);
    marker_array.markers.push_back(marker);
    ++idx;
    if (idx >= loop_size) {
      break;
    }
  }

  return marker_array;
}

MarkerArray createLaneChangingVirtualWallMarker(
  const geometry_msgs::msg::Pose & lane_changing_pose, const std::string & module_name,
  const rclcpp::Time & now, const std::string & ns)
{
  int32_t id{0};
  MarkerArray marker_array{};
  marker_array.markers.reserve(2);
  {
    auto wall_marker = createDefaultMarker(
      "map", now, ns + "virtual_wall", id, visualization_msgs::msg::Marker::CUBE,
      createMarkerScale(0.1, 5.0, 2.0), createMarkerColor(0.0, 1.0, 0.0, 0.5));
    wall_marker.pose = lane_changing_pose;
    wall_marker.pose.position.z += 1.0;
    marker_array.markers.push_back(wall_marker);
  }

  {
    auto text_marker = createDefaultMarker(
      "map", now, ns + "_text", id, visualization_msgs::msg::Marker::TEXT_VIEW_FACING,
      createMarkerScale(0.0, 0.0, 1.0), createMarkerColor(1.0, 1.0, 1.0, 1.0));
    text_marker.pose = lane_changing_pose;
    text_marker.pose.position.z += 2.0;
    text_marker.text = module_name;
    marker_array.markers.push_back(text_marker);
  }

  return marker_array;
}

}  // namespace marker_utils::lane_change_markers
