/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, SRI International
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of SRI International nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Luke Dennis */

// This is for interfacing with Moveit move group
#include <moveit/move_group_interface/move_group_interface.h>
// #include <moveit/planning_interface/move_group_interface.h>
// #include <moveit/planning_scene_interface/planning_scene_interface.h>

// These are for the various ROS message formats we need
#include <moveit_msgs/DisplayRobotState.h>
#include <moveit_msgs/DisplayTrajectory.h>

#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

// #include <moveit_msgs/AttachedCollisionObject.h>
// #include <moveit_msgs/CollisionObject.h>

int main(int argc, char** argv)
{
  ros::init(argc, argv, "move_group_interface_tutorial");
  auto nh = ros::NodeHandle{};
  auto spinner = ros::AsyncSpinner(1);
  spinner.start();

  std::cout << "Hit enter to start";
  std::cin.ignore();

  /* The planning group can be found in the ur5e_epick_moveit_config/config/ur5e.srdf */
  static const std::string PLANNING_GROUP = "manipulator";

  auto move_group = moveit::planning_interface::MoveGroupInterface(PLANNING_GROUP);

  // auto planning_scene_interface = moveit::planning_interface::PlanningSceneInterface();
  // auto planning_scene_interface = moveit::planning_interface::PlanningSceneInterface();

  auto const* joint_model_group = move_group.getCurrentState()->getJointModelGroup(PLANNING_GROUP);

  // We can print the name of the reference frame for this robot.
  ROS_INFO_NAMED("lab09", "Planning frame: %s", move_group.getPlanningFrame().c_str());

  // We can also print the name of the end-effector link for this group.
  ROS_INFO_NAMED("lab09", "End effector link: %s", move_group.getEndEffectorLink().c_str());

  // We can get a list of all the groups in the robot:
  ROS_INFO_NAMED("lab09", "Available Planning Groups:");
  std::copy(move_group.getJointModelGroupNames().begin(), move_group.getJointModelGroupNames().end(),
            std::ostream_iterator<std::string>(std::cout, ", "));

  // Get starting pose
  robot_state::RobotState start_state(*move_group.getCurrentState());

  // Generate target pose
  geometry_msgs::Pose target_pose1;
  target_pose1.position.x = 0.28;
  target_pose1.position.y = -0.2;
  target_pose1.position.z = 0.5;

  tf2::Quaternion quat;
  quat.setRPY(0,0,0);
  target_pose1.orientation = tf2::toMsg(quat);

  move_group.setPoseTarget(target_pose1);

  auto my_plan = moveit::planning_interface::MoveGroupInterface::Plan{};

  auto success = (move_group.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  if(!success) {
      ROS_WARN("Unable to plan path. Rerun or adjust target pose or planning parameters");
      return 1;
  }

  // Execute motion
  move_group.move();

  auto current_state = move_group.getCurrentState();

  auto joint_group_positions = std::vector<double>{};
  current_state->copyJointGroupPositions(joint_model_group, joint_group_positions);

  // Now, let's modify one of the joints, plan to the new joint space goal and visualize the plan.
  joint_group_positions[0] = -1.0;  // radians
  move_group.setJointValueTarget(joint_group_positions);

  success = (move_group.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  ROS_INFO_NAMED("tutorial", "Visualizing plan 2 (joint space goal) %s", success ? "" : "FAILED");
  move_group.move();

  //moveit_msgs::OrientationConstraint ocm;
  //ocm.link_name = "tool0";
  //ocm.header.frame_id = "base_link";
  //ocm.orientation.w = 1.0;
  //ocm.absolute_x_axis_tolerance = 0.1;
  //ocm.absolute_y_axis_tolerance = 0.1;
  //ocm.absolute_z_axis_tolerance = 0.1;
  //ocm.weight = 1.0;

  //// Now, set it as the path constraint for the group.
  //moveit_msgs::Constraints test_constraints;
  //test_constraints.orientation_constraints.push_back(ocm);
  //move_group.setPathConstraints(test_constraints);

  //// We will reuse the old goal that we had and plan to it.
  //// Note that this will only work if the current state already
  //// satisfies the path constraints. So, we need to set the start
  //// state to a new pose.
  //robot_state::RobotState start_state(*move_group.getCurrentState());
  //geometry_msgs::Pose start_pose2;
  //start_pose2.orientation.w = -1.0;
  //start_pose2.position.x = 0.55;
  //start_pose2.position.y = -0.05;
  //start_pose2.position.z = 0.8;
  //start_state.setFromIK(joint_model_group, start_pose2);
  //move_group.setStartState(start_state);

  //// Now we will plan to the earlier pose target from the new
  //// start state that we have just created.
  //move_group.setPoseTarget(target_pose1);

  //// Planning with constraints can be slow because every sample must call an inverse kinematics solver.
  //// Lets increase the planning time from the default 5 seconds to be sure the planner has enough time to succeed.
  //move_group.setPlanningTime(10.0);

  //success = (move_group.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  //ROS_INFO_NAMED("tutorial", "Visualizing plan 3 (constraints) %s", success ? "" : "FAILED");

  //// Visualize the plan in RViz
  //visual_tools.deleteAllMarkers();
  //visual_tools.publishAxisLabeled(start_pose2, "start");
  //visual_tools.publishAxisLabeled(target_pose1, "goal");
  //visual_tools.publishText(text_pose, "Constrained Goal", rvt::WHITE, rvt::XLARGE);
  //visual_tools.publishTrajectoryLine(my_plan.trajectory_, joint_model_group);
  //visual_tools.trigger();
  //visual_tools.prompt("next step");

  //// When done with the path constraint be sure to clear it.
  //move_group.clearPathConstraints();

  //// Cartesian Paths
  //// ^^^^^^^^^^^^^^^
  //// You can plan a Cartesian path directly by specifying a list of waypoints
  //// for the end-effector to go through. Note that we are starting
  //// from the new start state above.  The initial pose (start state) does not
  //// need to be added to the waypoint list but adding it can help with visualizations
  //std::vector<geometry_msgs::Pose> waypoints;
  //waypoints.push_back(start_pose2);

  //geometry_msgs::Pose target_pose3 = start_pose2;

  //target_pose3.position.z -= 0.2;
  //waypoints.push_back(target_pose3);  // down

  //target_pose3.position.y -= 0.2;
  //waypoints.push_back(target_pose3);  // right

  //target_pose3.position.z += 0.2;
  //target_pose3.position.y += 0.2;
  //target_pose3.position.x -= 0.2;
  //waypoints.push_back(target_pose3);  // up and left

  //// Cartesian motions are frequently needed to be slower for actions such as approach and retreat
  //// grasp motions. Here we demonstrate how to reduce the speed of the robot arm via a scaling factor
  //// of the maxiumum speed of each joint. Note this is not the speed of the end effector point.
  //move_group.setMaxVelocityScalingFactor(0.1);

  //// We want the Cartesian path to be interpolated at a resolution of 1 cm
  //// which is why we will specify 0.01 as the max step in Cartesian
  //// translation.  We will specify the jump threshold as 0.0, effectively disabling it.
  //// Warning - disabling the jump threshold while operating real hardware can cause
  //// large unpredictable motions of redundant joints and could be a safety issue
  //moveit_msgs::RobotTrajectory trajectory;
  //const double jump_threshold = 0.0;
  //const double eef_step = 0.01;
  //double fraction = move_group.computeCartesianPath(waypoints, eef_step, jump_threshold, trajectory);
  //ROS_INFO_NAMED("tutorial", "Visualizing plan 4 (Cartesian path) (%.2f%% acheived)", fraction * 100.0);

  //// Visualize the plan in RViz
  //visual_tools.deleteAllMarkers();
  //visual_tools.publishText(text_pose, "Joint Space Goal", rvt::WHITE, rvt::XLARGE);
  //visual_tools.publishPath(waypoints, rvt::LIME_GREEN, rvt::SMALL);
  //for (std::size_t i = 0; i < waypoints.size(); ++i)
  //  visual_tools.publishAxisLabeled(waypoints[i], "pt" + std::to_string(i), rvt::SMALL);
  //visual_tools.trigger();
  //visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to continue the demo");

  //// Adding/Removing Objects and Attaching/Detaching Objects
  //// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ////
  //// Define a collision object ROS message.
  //moveit_msgs::CollisionObject collision_object;
  //collision_object.header.frame_id = move_group.getPlanningFrame();

  //// The id of the object is used to identify it.
  //collision_object.id = "box1";

  //// Define a box to add to the world.
  //shape_msgs::SolidPrimitive primitive;
  //primitive.type = primitive.BOX;
  //primitive.dimensions.resize(3);
  //primitive.dimensions[0] = 0.4;
  //primitive.dimensions[1] = 0.1;
  //primitive.dimensions[2] = 0.4;

  //// Define a pose for the box (specified relative to frame_id)
  //geometry_msgs::Pose box_pose;
  //box_pose.orientation.w = 1.0;
  //box_pose.position.x = 0.4;
  //box_pose.position.y = -0.2;
  //box_pose.position.z = 1.0;

  //collision_object.primitives.push_back(primitive);
  //collision_object.primitive_poses.push_back(box_pose);
  //collision_object.operation = collision_object.ADD;

  //std::vector<moveit_msgs::CollisionObject> collision_objects;
  //collision_objects.push_back(collision_object);

  //// Now, let's add the collision object into the world
  //ROS_INFO_NAMED("tutorial", "Add an object into the world");
  //planning_scene_interface.addCollisionObjects(collision_objects);

  //// Show text in RViz of status
  //visual_tools.publishText(text_pose, "Add object", rvt::WHITE, rvt::XLARGE);
  //visual_tools.trigger();

  //// Wait for MoveGroup to recieve and process the collision object message
  //visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to once the collision object appears in RViz");

  //// Now when we plan a trajectory it will avoid the obstacle
  //move_group.setStartState(*move_group.getCurrentState());
  //geometry_msgs::Pose another_pose;
  //another_pose.orientation.w = 1.0;
  //another_pose.position.x = 0.4;
  //another_pose.position.y = -0.4;
  //another_pose.position.z = 0.9;
  //move_group.setPoseTarget(another_pose);

  //success = (move_group.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  //ROS_INFO_NAMED("tutorial", "Visualizing plan 5 (pose goal move around cuboid) %s", success ? "" : "FAILED");

  //// Visualize the plan in RViz
  //visual_tools.deleteAllMarkers();
  //visual_tools.publishText(text_pose, "Obstacle Goal", rvt::WHITE, rvt::XLARGE);
  //visual_tools.publishTrajectoryLine(my_plan.trajectory_, joint_model_group);
  //visual_tools.trigger();
  //visual_tools.prompt("next step");

  //// Now, let's attach the collision object to the robot.
  //ROS_INFO_NAMED("tutorial", "Attach the object to the robot");
  //move_group.attachObject(collision_object.id);

  //// Show text in RViz of status
  //visual_tools.publishText(text_pose, "Object attached to robot", rvt::WHITE, rvt::XLARGE);
  //visual_tools.trigger();

  ///* Wait for MoveGroup to recieve and process the attached collision object message */
  //visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to once the collision object attaches to the "
  //                    "robot");

  //// Now, let's detach the collision object from the robot.
  //ROS_INFO_NAMED("tutorial", "Detach the object from the robot");
  //move_group.detachObject(collision_object.id);

  //// Show text in RViz of status
  //visual_tools.publishText(text_pose, "Object dettached from robot", rvt::WHITE, rvt::XLARGE);
  //visual_tools.trigger();

  ///* Wait for MoveGroup to recieve and process the attached collision object message */
  //visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to once the collision object detaches to the "
  //                    "robot");

  //// Now, let's remove the collision object from the world.
  //ROS_INFO_NAMED("tutorial", "Remove the object from the world");
  //std::vector<std::string> object_ids;
  //object_ids.push_back(collision_object.id);
  //planning_scene_interface.removeCollisionObjects(object_ids);

  //// Show text in RViz of status
  //visual_tools.publishText(text_pose, "Object removed", rvt::WHITE, rvt::XLARGE);
  //visual_tools.trigger();

  ///* Wait for MoveGroup to recieve and process the attached collision object message */
  //visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to once the collision object disapears");

  //// END_TUTORIAL

  ros::shutdown();
  return 0;
}