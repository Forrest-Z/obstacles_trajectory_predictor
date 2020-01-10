#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <geometry_msgs/PoseArray.h>
#include <geometry_msgs/PoseStamped.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>

#include "obstacles_trajectory_predictor/obstacles_tracker.h"

class ObstaclesTrackerDemo
{
public:
    ObstaclesTrackerDemo(void);

    void obstacles_callback(const geometry_msgs::PoseArrayConstPtr&);
    void publish_velocity_arrows(std_msgs::Header);
    void publish_ids(std_msgs::Header);
    void process(void);

private:
    std::string FIXED_FRAME;

    ros::NodeHandle nh;
    ros::NodeHandle local_nh;
    ros::Publisher velocity_arrows_pub;
    ros::Publisher id_markers_pub;
    ros::Subscriber obstacles_sub;

    tf2_ros::Buffer tf_buffer;
    tf2_ros::TransformListener listener;

    ObstaclesTracker tracker;
};

ObstaclesTrackerDemo::ObstaclesTrackerDemo(void)
:local_nh("~"), listener(tf_buffer)
{
    velocity_arrows_pub = local_nh.advertise<visualization_msgs::MarkerArray>("velocity_arrows", 1);
    id_markers_pub = local_nh.advertise<visualization_msgs::MarkerArray>("id_markers", 1);
    obstacles_sub = nh.subscribe("/dynamic_obstacles", 1, &ObstaclesTrackerDemo::obstacles_callback, this);

    local_nh.param<std::string>("FIXED_FRAME", FIXED_FRAME, {"odom"});
}

void ObstaclesTrackerDemo::obstacles_callback(const geometry_msgs::PoseArrayConstPtr& msg)
{
    std::cout << "----- obstacles callback -----" << std::endl;
    geometry_msgs::PoseArray obstacle_pose;
    obstacle_pose = *msg;
    std::vector<Eigen::Vector2d> obstacles_position;
    try{
        geometry_msgs::TransformStamped transform;
        std::string frame_id = msg->header.frame_id;
        int slash_pos = frame_id.find('/');
        if(slash_pos == 0){
            frame_id.erase(0, 1);
        }
        transform = tf_buffer.lookupTransform(FIXED_FRAME, frame_id, ros::Time(0));
        for(auto& p : obstacle_pose.poses){
            geometry_msgs::PoseStamped p_;
            p_.header = obstacle_pose.header;
            p_.pose = p;
            // listener.transformPose(FIXED_FRAME, p_, p_);
            tf2::doTransform(p_, p_, transform);
            p = p_.pose;
            obstacles_position.emplace_back(Eigen::Vector2d(p.position.x, p.position.y));
        }
        obstacle_pose.header.frame_id = FIXED_FRAME;
    }catch(tf2::TransformException ex){
        std::cout << ex.what() << std::endl;
        return;
    }
    tracker.set_obstacles_position(obstacles_position);

    publish_velocity_arrows(msg->header);
    publish_ids(msg->header);
}

void ObstaclesTrackerDemo::publish_velocity_arrows(std_msgs::Header header)
{
    std::vector<Eigen::Vector2d> positions = tracker.get_positions();
    std::vector<Eigen::Vector2d> velocities = tracker.get_velocities();

    static int last_obs_num = 0;
    int obs_num = positions.size();
    visualization_msgs::MarkerArray velocity_arrows;
    for(int i=0;i<obs_num;i++){
        visualization_msgs::Marker v_arrow;
        v_arrow.header.stamp = header.stamp;
        v_arrow.header.frame_id = FIXED_FRAME;
        v_arrow.id = i;
        v_arrow.ns = "obstacles_tracker_demo/velocity_arrows";
        v_arrow.type = visualization_msgs::Marker::ARROW;
        v_arrow.action = visualization_msgs::Marker::ADD;
        v_arrow.lifetime = ros::Duration();
        v_arrow.pose.position.x = positions[i](0);
        v_arrow.pose.position.y = positions[i](1);
        double direction = atan2(velocities[i](1), velocities[i](0));
        tf2::Quaternion q;
        q.setRPY(0, 0, direction);
        v_arrow.pose.orientation = tf2::toMsg(q);
        v_arrow.scale.x = velocities[i].norm();
        v_arrow.scale.y = v_arrow.scale.z = 0.3;
        v_arrow.color.r = 1.0;
        v_arrow.color.a = 1.0;
        velocity_arrows.markers.push_back(v_arrow);
    }
    for(int i=obs_num;i<last_obs_num;i++){
        visualization_msgs::Marker v_arrow;
        v_arrow.id = i;
        v_arrow.ns = "obstacles_tracker_demo/velocity_arrows";
        v_arrow.action = visualization_msgs::Marker::DELETE;
        v_arrow.pose.orientation = tf2::toMsg(tf2::Quaternion(0, 0, 0, 1));
        velocity_arrows.markers.push_back(v_arrow);
    }
    velocity_arrows_pub.publish(velocity_arrows);
    last_obs_num = obs_num;
}

void ObstaclesTrackerDemo::publish_ids(std_msgs::Header header)
{
    std::vector<Eigen::Vector2d> positions = tracker.get_positions();
    std::vector<Eigen::Vector2d> velocities = tracker.get_velocities();
    std::vector<int> ids = tracker.get_ids();

    static int last_obs_num = 0;
    int obs_num = positions.size();
    visualization_msgs::MarkerArray id_markers;
    for(int i=0;i<obs_num;i++){
        visualization_msgs::Marker id_marker;
        id_marker.header.stamp = header.stamp;
        id_marker.header.frame_id = FIXED_FRAME;
        id_marker.id = i;
        id_marker.ns = "obstacles_tracker_demo/id_markers";
        id_marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
        id_marker.action = visualization_msgs::Marker::ADD;
        id_marker.lifetime = ros::Duration();
        id_marker.pose.position.x = positions[i](0);
        id_marker.pose.position.y = positions[i](1);
        id_marker.pose.position.z = 2.0;
        double direction = atan2(velocities[i](1), velocities[i](0));
        tf2::Quaternion q;
        q.setRPY(0, 0, direction);
        id_marker.pose.orientation = tf2::toMsg(q);
        id_marker.scale.z = 0.8;
        id_marker.color.r = 1.0;
        id_marker.color.g = 1.0;
        id_marker.color.b = 1.0;
        id_marker.color.a = 1.0;
        id_marker.text = std::to_string(ids[i]);
        id_markers.markers.push_back(id_marker);
    }
    for(int i=obs_num;i<last_obs_num;i++){
        visualization_msgs::Marker id_marker;
        id_marker.id = i;
        id_marker.ns = "obstacles_tracker_demo/id_markers";
        id_marker.action = visualization_msgs::Marker::DELETE;
        id_marker.pose.orientation = tf2::toMsg(tf2::Quaternion(0, 0, 0, 1));
        id_markers.markers.push_back(id_marker);
    }
    id_markers_pub.publish(id_markers);
    last_obs_num = obs_num;
}

void ObstaclesTrackerDemo::process(void)
{
    std::cout << "=== obstacles_tracker_demo ===" << std::endl;
    ros::spin();
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "obstacle_tracker");
    ObstaclesTrackerDemo obstacles_tracker_demo;
    obstacles_tracker_demo.process();
    return 0;
}
