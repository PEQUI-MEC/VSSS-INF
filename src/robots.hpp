/**
 * @file robots.hpp
 * @author Bryan Lincoln @ Pequi Mecânico
 * @date 19/05/2018
 * @brief Robot's Manager Singleton Class
 * @see https://www.facebook.com/NucleoPMec/
 * @sa https://www.instagram.com/pequimecanico
 */

#ifndef ROBOTS_HPP_
#define ROBOTS_HPP_
#include "opencv2/opencv.hpp"

#define check_index(...) _check_index(__FUNCTION__, __VA_ARGS__)
#define N_ROBOTS 3

class Robots {
// ATTRIBUTES
public:
    /// The number of robots. Can be used by all classes statically or dynamically.
    static const int SIZE = N_ROBOTS;

    /// Robot's physical maximum velocity in meters/second. Can be used by all classes statically or dynamically.
    static constexpr float MAX_VEL = 1.4f;

    // ** Must be declared here, before 'struct Robot'
    /**
     * All available types of command that can be sent to robot.
     * This refers to robot's intrinsics and must be encapsulated inside the robot's class.
     */
     enum CMD {
        POSITION,
        SPEED,
        ORIENTATION,
        VECTOR
    };

    /// A structure to communicate Robot's Status.
    struct _Status {
        /**
        *  Robot's position.
        *  (-1, -1) means that it has not been set yet.
        *  The robot's last position is maintained if we can't determine it in the actual frame.
        *  Remember: Y grows down, not up. X behaves normally.
        */
        cv::Point position = cv::Point(-1, -1);

        /**
         * Robot's orientation.
         * 0 (or -0) tells us the robot is facing the opponent goal directly.
         * 180 (or -180) tells us the robot is facing our goal directly.
         * -90 tells us the robot is facing the upper side (smaller values of Y) of the field.
         * 90 tells us the robot is facing the down side (higher values of Y) of the field.
         */
        double orientation = 0;

        /// Robot's maximum velocity
        float vmax = 0;
    };

    /// A structure to encapsulate and communicate Robot commands.
    struct Command {
        /// The hardware ID for the commanded robot
        char ID = 'A';

        /// This command's type (CMD_*)
        CMD cmdType = CMD::POSITION;

        /// This robot's status
        _Status Status;

        /// The message (would be nice to use Union data structure but cv::Point has a constructor)
        struct _Msg {
            /// CMD_POSITION target position
            cv::Point target = cv::Point(-1, -1);

            /// CMD_VECTOR or CMD_ORIENTATION target orientation
            double theta = 0;

            /// CMD_SPEED target wheel velocities
            struct _Velocity {
                float right = 0;
                float left = 0;
            } Velocity;
        } Msg;
    };

private:
    /**
     * An internal structure to keep robot information.
     * This structure cannot be seen by other classes - use encapsulated
     * objects (like Robots::Command or Robots::_Status) to communicate Robot's information.
     */
    struct Robot {
        /**
         *  Robot's position.
         *  (-1, -1) means that it has not been set yet.
         *  The robot's last position is maintained if we can't determine it in the actual frame.
         *  Remember: Y grows down, not up. X behaves normally.
         */
        cv::Point position = cv::Point(-1, -1);

        /// Robot's front point
        cv::Point secondary = cv::Point(-1, -1);

        /**
         * Robot's CMD_POSITION target position
         * (-1, -1) means that the target is not set.
         */
        cv::Point target = cv::Point(-1, -1);

        /**
         * Robot's orientation.
         * 0 (or -0) tells us the robot is facing the opponent goal directly.
         * 180 (or -180) tells us the robot is facing our goal directly.
         * -90 tells us the robot is facing the upper side (smaller values of Y) of the field.
         * 90 tells us the robot is facing the down side (higher values of Y) of the field.
         */
        double orientation = 0;

        /// Robot's CMD_ORIENTATION target orientation
        double targetOrientation = 0;

        /// Robot's role for strategy
        int role = 0;

        /// Robot's CMD_VECTOR angle to its current orientation
        double transAngle = 0;

        /// Robot's command type, using CMD_* constants
        CMD cmdType = CMD::POSITION;

        /// Robot's maximum velocity
        float vmax = 0; // m/s

        /// Robot's CMD_SPEED left and right wheel's velocity
        float Vl = 0, Vr = 0;  // m/s

        /// Indicates if this robot has a fixed position (same position over time does not indicate collision)
        bool fixedPos = false;

        /// Indicates if this robot is using potential field
        bool usingPotField = false; // !TODO remover essa variável e usar cmdType pra verificar se comando já foi convertido pra vector

        /// Robot's status defined by strategy
        int status = 0;

        /// This robot's hardware ID
        char ID = 'A';
    };

    /// A list of N robots. Must be unique in the entire program.
    std::vector<Robot> list;

    /// The default velocities for each role (GK, DEF or ATK)
    float default_vel[N_ROBOTS];

    /// The unique instance of this class
    static Robots * instance;

    /// Initializes our arrays
    Robots();

    /**
     * Returns an existing Robots instance or creates one.
     * @return an instance of robots.
     */
    static Robots * get_instance();

    /**
     * Checks if a given index is valid considering the number of robots.
     * !TODO convert all indexes to an unsigned type
     * @param caller The name of the method that called Robot::_check_index. This is set automatically by #define directives.
     * @param i An index to validate.
     * @return True if 'i' is valid, False otherwise.
     */
    static bool _check_index(const std::string& caller, int i);

// METHODS
public:
    /// GETTERS

    /**
     * Encapsulate robots' commands and status to send via Communication.
     * !TODO see if there is any kind of desynchronization when this is encapsulating robot data and Vision is updating our position, for example.
     * @return An array of Commands, one for each robot.
     */
    static std::vector<Command> get_commands();

    /**
     * Gets a robot's hardware ID.
     * @param robot Robot's index.
     * @return A char representing this robot's hardware ID.
     */
    static char get_ID(int robot);

    /**
     * Tells us if a given robot is using potential field.
     * @param robot Robot's index.
     * @return True if it is using potential field, False otherwise.
     */
    static bool get_usingPotField(int robot);

    /**
     * Gets a given robot's position.
     * @param robot Robot's index.
     * @return A point representing its position on the screen. Remember: Y grows down, not up.
     */
    static cv::Point get_position(int robot);

    /**
     * Gets a given robot's target.
     * @param robot Robot's index.
     * @return A point representing its target on the screen. Remember: Y grows down, not up.
     */
    static cv::Point get_target(int robot);

    /**
     * Gets a given robot's orientation.
     * @param robot Robot's index.
     * @return A double precision decimal value representing it's orientation.
     */
    static double get_orientation(int robot);

    /**
     * Gets the secondary tag position (it's front point) of a given robot.
     * @param robot Robot's index.
     * @return A point representing robot's front position. Remember: Y grows down, not up.
     */
    static cv::Point get_secondary_tag(int robot);

    /**
     * Tells us if a given robot has a fixed position (i.e. strategy's collision check will ignore this one).
     * @param robot Robot's index.
     * @return True if this robot has a fixed position, False otherwise.
     */
    static bool get_fixedPos(int robot);

    /**
     * Gets the command type of a given robot.
     * @param robot Robot's index.
     * @return An enum CMD value.
     */
    static CMD get_cmdType(int robot);

    /**
     * Gets a given robot's (dynamically set) maximum velocity. This value may be changed by strategy at runtime.
     * @param robot Robot's index.
     * @return This robot's current maximum velocity.
     */
    static float get_velocity(int robot);

    /**
     * Gets a given robot's untouched maximum velocity.
     * This value is not changed by strategy at runtime and can be used as an acceleration base.
     * @param robot Robot's index.
     * @return This robot's default maximum velocity.
     */
    static float get_default_velocity(int robot);

    /**
     * Gets a given role's default maximum velocity. This is set by user interface.
     * @param role Role's index.
     * @return This role's default maximum velocity.
     */
    static float get_default_role_velocity(int role);

    /**
     * Gets a given robot's role.
     * The returned value is managed by strategy.
     * @param robot Robot's index.
     * @return An int representing this robot's role.
     */
    static int get_role(int robot);

    /**
     * Gets a given robot's status.
     * The returned value is managed by strategy.
     * @param robot Robot's index.
     * @return An int representing this robot's status.
     */
    static int get_status(int robot);

    /**
     * Gets a given robot's transposed vector angle.
     * Considering this robot's position and orientation as reference,
     * this value represents the direction this robot must follow.
     * @param robot Robot's index
     * @return A double-precision decimal value representing this robot's transposed vector angle.
     */
    static double get_transAngle(int robot);

    /// SETTERS

    /**
     * Sets a given robot's hardware ID.
     * @param robot Robot's index.
     * @param ID The new ID to be set.
     */
    static void set_ID(int robot, char ID);

    /**
     * Sets a given robot's position.
     * @param robot Robot's index.
     * @param position A point representing the new robot's position.
     */
    static void set_position(int robot, cv::Point position);

    /**
     * Sets a given robot's orientation.
     * @param robot Robot's index.
     * @param orientation A double-precision decimal number representing the robot's new orientation.
     */
    static void set_orientation(int robot, double orientation);

    /**
     * Sets a given robot's secondary tag (robot's front point).
     * @param robot Robot's index.
     * @param secondary A point representing it's new secondary point.
     */
    static void set_secondary_tag(int robot, cv::Point secondary);

    /**
     * Sets a given robot's command type.
     * @param robot Robot's index.
     * @param cmdType An enum CMD value representing the new command type.
     */
    static void set_cmdType(int robot, CMD cmdType);

    /**
     * Sets a given robot's fixed position status.
     * @param robot Robot's index.
     * @param fixedPos Is this robot's position fixed?.
     */
    static void set_fixedPos(int robot, bool fixedPos);

    /**
     * Sets a given robot's 'using potential field' status.
     * @param robot Robot's index.
     * @param usingPotField Is this robot's using potential field?.
     */
    static void set_usingPotField(int robot, bool usingPotField);

    /**
     * Sets a given robot's maximum velocity to a given role's default.
     * @param robot Robot's index.
     * @param role A role that will be the reference to the robot's velocity.
     */
    static void set_velocity(int robot, int role);

    /**
     * Sets a given robot's maximum velocity to a given value.
     * @param robot Robot's index.
     * @param velocity The robot's new velocity.
     */
    static void set_velocity(int robot, float velocity);

    /**
     * Resets a given robot's velocity to it's role's default.
     * @param robot Robot's index.
     */
    static void set_velocity(int robot);

    /**
     * Sets a given robot's role a new default velocity.
     * @param robot Robot's index.
     * @param vel The role's new default velocity.
     */
    static void set_default_velocity(int robot, float vel);

    /**
     * Sets a given role's default velocity.
     * @param role Role's index.
     * @param vel The role's new default velocity.
     */
    static void set_default_role_velocity(int role, float vel);

    /**
     * Sets a given robot's status.
     * This value is managed by strategy.
     * @param robot Robot's index.
     * @param status The new robot's status.
     */
    static void set_status(int robot, int status);

    /**
     * Sets a given robot's role.
     * This value is managed by strategy.
     * @param robot Robot's index.
     * @param role The new robot's role.
     */
    static void set_role(int robot, int role);

    /**
     * Sets a given robot's transposed vector angle.
     * Considering this robot's position and orientation as reference,
     * this value represents the direction this robot must follow.
     * @param robot Robot's index.
     * @param transAngle The new robot's transposed vector angle.
     */
    static void set_transAngle(int robot, double transAngle);

    /**
     * Sets a given robot's target position.
     * @param robot Robot's index.
     * @param target The new robot's target position.
     */
    static void set_target(int robot, cv::Point target);

    /**
     * Sets a given robot's CMD::POSITION command.
     * This changes it's command type, target position and maximum velocity.
     * @param robot Robot's index.
     * @param target The new robot's target position.
     * @param vel (optional) Robot's velocity while executing this command.
     *                       (If not specified, the default maximum velocity for
     *                        it's role is used.)
     */
    static void set_command(int robot, cv::Point target, float vel = -1);

    /**
     * Sets a given robot's CMD::SPEED command.
     * This changes it's command type, general maximum velocity and each wheel's maximum velocity.
     * This is the robot's most primitive command.
     * @param robot Robot's index.
     * @param Vl New maximum left wheel velocity.
     * @param Vr New maximum right wheel velocity.
     */
    static void set_command(int robot, float Vl, float Vr);

    /**
     * Sets a given robot's CMD::VECTOR command.
     * This changes it's command type, transposed target vector angle and maximum velocity.
     * @param robot Robot's index.
     * @param theta The new robot's transposed target vector angle.
     *              (This should not be used to send CMD::ORIENTATION commands. For this, use look_at() method.)
     * @param vel (optional) Robot's velocity while executing this command.
     *                       (If not specified, the default maximum velocity for
     *                        it's role is used.)
     */
    static void set_command(int robot, double theta, float vel = -1);


    /**
     * Resets all robots' CMD::POSITION target positions by setting them to (-1, -1).
     */
    static void reset_targets();

    /**
     * Resets all robots' velocities, targets and sets command types to CMD::VECTOR.
     */
    static void reset_commands();


    /**
     * Checks if a given robot's position is set.
     * Note: the default unset position is (-1, -1).
     * @param robot Robot's index.
     * @return True if this robot's position is set, False otherwise.
     */
    static bool is_posistion_set(int robot);

    /**
     * Checks if a given robot's target position is set.
     * Note: the default unset position is (-1, -1).
     * @param robot Robot's index.
     * @return True if this robot's target position is set, False otherwise.
     */
    static bool is_target_set(int robot);

    /**
     * Stops a robot by sending a CMD::SPEED where Vl = Vr = 0, resetting target position and setting maximum velocity to 0.
     * @param robot Robot's index.
     */
    static void stop(int robot);

    /**
     * Commands a robot to spin clockwiselly.
     * To do this, it sends a CMD::SPEED with Vl = vel and Vr = -vel.
     * @param robot Robot's index.
     * @param vel (optional) Robot's velocity while executing this command.
     *                       (If not specified, the default maximum velocity for
     *                       it's role is used.)
     */
    static void spin_clockwise(int robot, float vel = -1);

    /**
     * Commands a robot to spin counter-clockwiselly.
     * To do this, it calls spin_counter_clockwise(robot, -vel).
     * @param robot Robot's index.
     * @param vel (optional) Robot's velocity while executing this command.
     *                       (If not specified, the default maximum velocity for
     *                        it's role is used.)
     */
    static void spin_counter_clockwise(int robot, float vel = -1);

    /**
     * Sends a CMD::ORIENTATION command to turn to a given orientation.
     * @param robot Robot's index.
     * @param orientation The robot's target orientation.
     * @param vel (optional) Robot's velocity while executing this command.
     *                       (If not specified, the default maximum velocity for
     *                        it's role is used.)
     */
    static void look_at(int robot, double orientation, float vel = -1);

    /**
     * Sends a CMD::ORIENTATION command to face a given point.
     * @param robot Robot's index.
     * @param target The point that this robot must turn to.
     * @param vel (optional) Robot's velocity while executing this command.
     *                       (If not specified, the default maximum velocity for
     *                        it's role is used.)
     */
    static void look_at(int robot, cv::Point target, float vel = -1);

    /**
     * Goes at maximum velocity to a given target using CMD::VECTOR.
     * @param robot Robot's index.
     * @param target The robot's kick target.
     */
    static void kick(int robot, cv::Point target);
};

#endif /* ROBOTS_HPP_ */
