#include "ros/ros.h"
#include "std_msgs/String.h"
#include <geometry_msgs/Twist.h>

#include <boost/asio.hpp>
#include <sstream>


using namespace boost;

#define PORT "/dev/ttyACM1"



void speedCallback(const std_msgs::String::ConstPtr& msg){
    ROS_INFO("I heard: [%s]", msg->data.c_str());
}

char read_char(asio::serial_port &port){
	char c;
	// Read 1 character into c, this will block
	// forever if no character arrives.
    try{
	asio::read(port, asio::buffer(&c,1));
    return c;
        
    }
    catch(boost::system::system_error e){
    return '\0';
    }
	
}



int main(int argc, char **argv)
{
    asio::io_service io;
    asio::serial_port port(io);

    port.open(PORT);
    port.set_option(asio::serial_port_base::baud_rate(115200));
    port.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

  ros::init(argc, argv, "comms");

  ros::NodeHandle n;
  ros::Publisher pos_pub = n.advertise<std_msgs::String>("position", 10);
  ros::Publisher turtle_pub = n.advertise<geometry_msgs::Twist>("/turtle1/cmd_vel", 100);
  ros::Subscriber sub = n.subscribe("speed", 1000, speedCallback);

  ros::Rate loop_rate(5000);

   std_msgs::String msg;
    std::stringstream ss;
  bool flag = false;
  while (ros::ok()) //only runs when roscore == TRUE
  {
    while(!flag){// while no new data
    char c = read_char(port); // runs serial read for one character
    if(c == '\0'){
        //failed to get data and skip adding
    }
    else if (c== '\n'){
        flag = true; // when end of transmission EOT
    }
    else{
        ss << c; // stream c character into the stringstream 
    }
    }
    
    msg.data = ss.str();
    int n = std::stoi(ss.str());
    //ROS_INFO("%s", msg.data.c_str());
    
    // CHECK TO COMMUNICATE WITH TURTLE SIM 
    if(n < 20){
        geometry_msgs::Twist tmsg;
        tmsg.linear.x = 2.0;
        turtle_pub.publish(tmsg);
    }
    if(n > 400 && n < 420){
        geometry_msgs::Twist tmsg;
        tmsg.linear.x = -2.0;
        turtle_pub.publish(tmsg);
    }
    
    if(n > 250 && n < 270){
        geometry_msgs::Twist tmsg;
        tmsg.angular.z = -2.0;
        turtle_pub.publish(tmsg);
    }
    if(n > 90 && n < 110){
        geometry_msgs::Twist tmsg;
        tmsg.angular.z = 2.0;
        turtle_pub.publish(tmsg);
    }
    
    // Publish msg
    pos_pub.publish(msg);
    ss.str(std::string()); // reset the ss to nothing
    flag = false; // reset the new data flag
    ros::spinOnce(); // allow callbacks
    loop_rate.sleep(); // sleep using ros::Rate loop_rate
  }

port.close();
  return 0;
}
