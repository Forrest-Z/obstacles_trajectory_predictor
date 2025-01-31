#ifndef __OBSTACLES_TRACKER_H
#define __OBSTACLES_TRACKER_H

#include <boost/optional.hpp>

#include "obstacles_trajectory_predictor/obstacle.h"
#include "obstacles_trajectory_predictor/social_force_model.h"

class ObstaclesTracker
{
public:
    typedef std::map<int, Obstacle> ObstaclesWithID;;

    ObstaclesTracker(void);

    void set_obstacles_position(const std::vector<Eigen::Vector2d>&);
    void set_static_obstacles_position(const std::vector<Eigen::Vector2d>&);
    void setup_simulation(const std::vector<Obstacle>&);
    std::vector<Eigen::Vector2d> get_velocities(void);
    std::vector<Eigen::Vector2d> get_positions(void);
    std::vector<int> get_ids(void);
    std::vector<Obstacle> get_obstacles(const ObstaclesWithID&);
    std::vector<Obstacle> get_obstacles(void);
    std::vector<Obstacle> get_moving_obstacles(void);
    std::vector<Obstacle> simulate_one_step(const std::vector<Obstacle>&);
    void set_verbose_output(bool);
    bool solve_hungarian_method(Eigen::MatrixXi&, std::vector<int>&);

private:
    bool associate_obstacles(const std::vector<Eigen::Vector2d>&, std::vector<int>&);
    double get_distance(const Obstacle&, const Eigen::Vector2d&);
    int get_id_from_index(int);
    int get_new_id(void);
    void update_tracking(const std::vector<Eigen::Vector2d>&, const std::vector<int>&);

    double SAME_OBSTACLE_THRESHOLD;
    double ERASE_LIKELIHOOD_THREHSOLD;
    double NOT_OBSERVED_TIME_THRESHOLD;
    double DEFAULT_LIFE_TIME;
    double DT;
    double MOVING_THRESHOLD;
    double MAX_SOCIAL_FORCE_THREHOLD;// too much force may be due to misrecognition
    bool VERBOSE;

    ObstaclesWithID obstacles;
    std::vector<Eigen::Vector2d> static_obstacles;
    std::vector<size_t> misrecognition_ids;
    SocialForceModel sfm;
};

#endif// __OBSTACLES_TRACKER_H
