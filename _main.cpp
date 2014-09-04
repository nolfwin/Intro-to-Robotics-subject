
#include <opencv2\opencv.hpp>
#include "CreateConnector.h"

CreateConnector robot;
CreateData data;

int main()
{
	int velL = 0;
	int velR = 0;

	robot.Connect("COM7");

	cv::namedWindow("hare");

	while(true)
	{
		robot.DriveDirect(velL, velR);
		if( robot.ReadData(data) )
			std::cout << "data : " << data.cliffSignal[0] << " " << data.cliffSignal[1] << " " << data.cliffSignal[2] << " " << data.cliffSignal[3] << std::endl;

	//	velL *= 0.5;
	//	velR *= 0.5;

		char c = cv::waitKey(30);
		if( c == 27 )
			break;


	/*	if(data.cliffSignal[0]+data.cliffSignal[1]+data.cliffSignal[2]+data.cliffSignal[3] <3200){
			velL = 0; velR = 0;
		}*/
		if(data.cliffSignal[0]<800 ){
				 velL = -50; 
				 velR = 200;
		}
		else if(data.cliffSignal[3]>1200){
			velL = 200; 
				 velR = -50;
		}
		else if(data.cliffSignal[1]<550 ){
				 velL = -50; 
				 velR = 200;
		}
		else if(data.cliffSignal[2]>1200 ){
				 velL = 200; 
				 velR = -50;
		}
		else if(data.bumper[0] || data.bumper[1]){
			
			robot.DriveDirect(-500, -500);
			c = cv::waitKey(1000);
			robot.DriveDirect(-500, 500);
			c = cv::waitKey(500);
		}
		else {
			velL = 500; velR = 500;
		}

		//switch(c){
		//case 'w': velL = 500; velR = 500; break;
		//case 's': velL = -500; velR = -500; break;
		//case 'a': velL = -500; velR = 500; break;
		//case 'd': velL = 500; velR = -500; break;
		//case 'x': velL = velR = 0; break;
		//}

	}
robot.Disconnect();

	return 0;
}