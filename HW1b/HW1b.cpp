// HW1b.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "EasyBMP/EasyBMP.h"
#include <iostream>
#include <fstream>
#include <string>
#include <atlstr.h>
#include <sstream>
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "perlin/perlin.h"

using namespace std;
//using namespace EasyBMP;

int writeValuess(int);
int* findSpace(const char*);
float calDistance(float*, float*);
int compareLargest(int, float*);
int compareSmallest(int, float*);
void createDensity(int, bool, int[], float, float[], float[], float, float[], Perlin);
float createLightSource(float*, float*, int, float*, float, float, float, int*, float*, bool);
float doTriLinear(float*, float*, float, int, int*, float, float*);
wstring StringToWstring(const string);
string WstringToString(const wstring);
int compare(int, int, int);

int main(int argc, char* argv[])
{
	//configure file parameter
	float DELT, STEP, FOVY;
	int wide, tall, deep;
	int XYZC[3], resolution[2];
	float bgColor[3], materialColor[3], eyePosition[3], vDir[3], UVEC[3], lightPos[3], lightColor[3], center[3], radius;
	int *typeArray;
	float *centerArray;
	float *radiusArray;
	int objectCount;
	float k = 1;
	float *densityArray; 
	float *lightArray;

	//second light source
	float *lightArray2;
	float lightPos2[3] = {-200,0,0};
	float lightColor2[3] = {0,0,0.8};

	//third light source
	float *lightArray3;
	float lightPos3[3] = {0, 200, 0};
	float lightColor3[3] = {1,1,1};

	//To store all the light position
	float *arrLightPos[3];
	arrLightPos[0] = lightPos;
	arrLightPos[1] = lightPos2;
	arrLightPos[2] = lightPos3;

	//To store all the light color
	float *arrLightColor[3];
	arrLightColor[0] = lightColor;
	arrLightColor[1] = lightColor2;
	arrLightColor[2] = lightColor3;

	float *arrLightArray[3];

	float bgColor2[3] = {1,1,1};//create gradient background color

	bool doMultiMaterialColor = true;//Create image with multi material color
	float materialColor2[3] = {0.7,0.7,0.7};//Build second material color
	float *arrMaterialColor[2];
	arrMaterialColor[0] = materialColor;
	arrMaterialColor[1] = materialColor2;

	//perlin noise parameter
	int octave = 6;
	float freq = 0.05;
	float amp = 0;
	int seed = 3;

	string saveFileName;
	bool doInterpolation = true;	  //Do ray march with tri-linear interpolation
	bool doTriFilter = true;		  //Create density with Tri-Filter

	float coordOrigin[3] = {0, 0, 0};
	char c;
	int objectNum = 0;
	string type;
	string confgFileName;


	//input config file name
	if(argv[1] != NULL)
			confgFileName = argv[1];
	else
	{
		cout << "Type the file name :" << endl;
		cin >> confgFileName;
	}


	//cout << "Your are rendering the <" << confgFileName << "> file." << endl; 
	//cout << endl;
	//cout << "Do you want to do the tri-linear interpolation (Y/N)?" << endl;
	//cin >> c;

	//if(c == 'y' || c == 'Y')
	//	doInterpolation = true;
	//else
		doInterpolation = false;


	//cout << "Do you want to create density with tri-Filter (Y/N)?" << endl;
	//cin >> c;

	//if(c == 'y' || c == 'Y')
		doTriFilter = true;
	//else
		//doTriFilter = false;

	//get current file directory
	_TCHAR szEXEPath1[2048];
	GetModuleFileName( NULL, szEXEPath1, 2048);//catch current path must include<Windows.h>
	wstring wpath = szEXEPath1;
	string path = WstringToString(wpath);
	string newPath = "";
	int slashCount = 0;
	for(int j = 1; j <=path.length(); j++)
	{
		if(path.at(path.length()-j) == '\\')
		{
			slashCount++;

			if(slashCount == 2)
				newPath = path.substr(0, path.length() - j + 1);
		}	
	}

	ifstream inFile(newPath + confgFileName);//read file content


	//read file
	string line;
	int arrayIndex = 0;
	int lineIndex = 0;
	bool readLineFlag = true;
	bool loadValueFlag = true;
	cout<< "Loading file" << endl;
	#pragma region //Read File
	while(getline(inFile,line))
	{
		string str = "";
		string restStr = "";
		if(readLineFlag == true)
		{
			readLineFlag = false;
			for(int i =0 ; i < line.length() ; i++)
			{
				if(line.at(i) == ' ')//If there is space
				{
					str = line.substr(0,i);
					restStr = line.substr(i+1);
					if(str == "DELT")//float
					{
						readLineFlag = true;
						cout << "Loading DELT..." << endl;
						DELT =  atof(restStr.c_str());//Convert String to value  caution need to add .c_str()
						break;
					}
					else if(str == "STEP")//float
					{
						readLineFlag = true;
						cout << "Loading STEP..." << endl;
						STEP =  atof(restStr.c_str());//Convert String to value  caution need to add .c_str()
						break;
					}
					else if(str == "XYZC")//integer
					{
						readLineFlag = true;
						cout << "Loading XYZC..." << endl;
						const char* restCharArr = restStr.c_str();
						//cout << "XYZC string = " << restStr << "end" << endl;
						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}

						stringstream ss1,ss2;
						ss1<< index1;
						ss2<< index2;
						//cout << "XYZC index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<   endl;
						XYZC[0] = atoi(restStr.substr(0, index1).c_str());//Convert String to value  caution need to add .c_str()
						XYZC[1] = atoi(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						XYZC[2] = atoi(restStr.substr(index2 + 1).c_str());

						densityArray = new float[XYZC[0] * XYZC[1] * XYZC[2]];
						lightArray = new float[XYZC[0] * XYZC[1] * XYZC[2]];
						lightArray2 = new float[XYZC[0] * XYZC[1] * XYZC[2]];
						lightArray3 = new float[XYZC[0] * XYZC[1] * XYZC[2]];
						for(int i =0 ; i < XYZC[0] * XYZC[1] * XYZC[2] ; i++)
						{
							densityArray[i] = 0;
							lightArray[i] = -1;
							lightArray2[i] = -1;
							lightArray3[i] = -1;
						}
						arrLightArray[0] = lightArray;
						arrLightArray[1] = lightArray2;
						arrLightArray[2] = lightArray3;
						break;
					}
					else if(str == "BRGB")//float
					{
						readLineFlag = true;
						cout << "Loading BRGB..." << endl;
						const char* restCharArr = restStr.c_str();
						//cout << "BRGB string = " << restStr << "end" << endl;						
						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}
						stringstream ss1,ss2;
						ss1<< index1;
						ss2<< index2;


						//cout << "BRGB index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<   endl;
						bgColor[0] = atof(restStr.substr(0, index1).c_str());
						bgColor[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						bgColor[2] = atof(restStr.substr(index2 + 1).c_str());
						break;
					}
					else if(str == "MRGB")//float
					{
						readLineFlag = true;
						cout << "Loading MRGB..." << endl;
						const char* restCharArr = restStr.c_str();
						//cout << "MRGB string = " << restStr << "end" << endl;
						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}
						int length = restStr.length();

						stringstream ss1,ss2,ss3;
						ss1<< index1;
						ss2<< index2;
						ss3<< length;

						//cout << "MRGB index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<" = "<< ss3.str()<<   endl;
						materialColor[0] = atof(restStr.substr(0, index1).c_str());
						materialColor[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						materialColor[2] = atof(restStr.substr(index2 + 1).c_str());
						break;
					}
					else if(str == "FILE")
					{
						readLineFlag = true;
						cout << "Loading FILE..." << endl;
						saveFileName = restStr;
						break;
					}
					else if(str == "RESO")//integer
					{
						readLineFlag = true;
						cout << "Loading RESO..." << endl;
						int index1 = 0;
						for(int j =0 ; j < restStr.length() ; j++)
						{
							if(restStr.at(j) == ' ')
								index1 = j;
						}
						resolution[0] = atoi(restStr.substr(0, index1).c_str());
						resolution[1] = atoi(restStr.substr(index1 + 1).c_str());
						break;
					}
					else if(str == "EYEP")//float
					{
						readLineFlag = true;
						cout << "Loading EYEP..." << endl;
						const char* restCharArr = restStr.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}
						stringstream ss1,ss2;
						ss1<< index1;
						ss2<< index2;
						//cout << "EYEP index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<   endl;
						eyePosition[0] = atof(restStr.substr(0, index1).c_str());
						eyePosition[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						eyePosition[2] = atof(restStr.substr(index2 + 1).c_str());
						break;
					}
					else if(str == "VDIR")//float
					{
						readLineFlag = true;
						cout << "Loading VDIR..." << endl;
						const char* restCharArr = restStr.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}
						stringstream ss1,ss2;
						ss1<< index1;
						ss2<< index2;
						//cout << "VDIR index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<   endl;
						vDir[0] = atof(restStr.substr(0, index1).c_str());
						vDir[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						vDir[2] = atof(restStr.substr(index2 + 1).c_str());
						break;
					}
					else if(str == "UVEC")//float
					{
						readLineFlag = true;
						cout << "Loading UVEC..." << endl;
						const char* restCharArr = restStr.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}
						stringstream ss1,ss2;
						ss1<< index1;
						ss2<< index2;
						//cout << "UVEC index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<   endl;
						UVEC[0] = atof(restStr.substr(0, index1).c_str());
						UVEC[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						UVEC[2] = atof(restStr.substr(index2 + 1).c_str());
						break;
					}
					else if(str == "FOVY")
					{
						readLineFlag = true;
						cout << "Loading FOVY..." << endl;
						FOVY =  atof(restStr.c_str());
						break;
					}
					else if(str == "LPOS")//float
					{
						readLineFlag = true;
						cout << "Loading LPOS..." << endl;
						const char* restCharArr = restStr.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}
						stringstream ss1,ss2;
						ss1<< index1;
						ss2<< index2;
						//cout << "LPOS index1 = " << ss1.str() << "  index2 = " <<ss2.str()<<   endl;
						lightPos[0] = atof(restStr.substr(0, index1).c_str());
						lightPos[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						lightPos[2] = atof(restStr.substr(index2 + 1).c_str());


						break;
					}
					else if(str == "LCOL")//float
					{
						readLineFlag = true;
						cout << "Loading LCOL..." << endl;
						const char* restCharArr = restStr.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}

						lightColor[0] = atof(restStr.substr(0, index1).c_str());
						lightColor[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						lightColor[2] = atof(restStr.substr(index2 + 1).c_str());


						break;
					}
					else if(str == "ORIG")
					{
						readLineFlag = true;
						cout << "Loading ORIG..." << endl;
						const char* restCharArr = restStr.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}

						coordOrigin[0] = atof(restStr.substr(0, index1).c_str());
						coordOrigin[1] = atof(restStr.substr(index1 + 1, index2 - index1 - 1).c_str());
						coordOrigin[2] = atof(restStr.substr(index2 + 1).c_str());
						break;
					}
				}
			}
		}


		if(readLineFlag == false)
		{
			if(lineIndex == 14)
			{
				objectCount = atoi(line.c_str());
				typeArray = new int[objectCount];
				centerArray = new float[objectCount * 3];
				radiusArray = new float[objectCount];
			}

			if(lineIndex > 15) 
			{
				if(lineIndex % 4 == 0)
				{
					if(line == "sphere")
						typeArray[objectNum] = 0;
					else if(line == "cloud")
						typeArray[objectNum] = 1;
					else if(line == "pyroclastic")
						typeArray[objectNum] = 2;
					else if(line == "quad")
						typeArray[objectNum] = 3;
					else if(line == "quadcloud" || line == "Quadcloud" || line == "quadCloud" || line == "QuadCloud")
						typeArray[objectNum] = 4;
				}
				if(lineIndex % 4 == 1)
				{
					const char* restCharArr = line.c_str();

						int index1 = 0;
						int index2 = 0;

						for(int j = 0; j < strlen(restCharArr) ; j++)
						{
							if(restCharArr[j] == ' ')
							{
								if(index1 == 0)
									index1 = j;
								else
									index2 = j;
							}
						}

					centerArray[objectNum * 3 + 0] = atof(line.substr(0, index1).c_str());
					centerArray[objectNum * 3 + 1] = atof(line.substr(index1 + 1, index2 - index1 - 1).c_str());
					centerArray[objectNum * 3 + 2] = atof(line.substr(index2 + 1).c_str());

				}
				if(lineIndex % 4 == 2)
				{
					radiusArray[objectNum] = atof(line.c_str());
					objectNum++;
				}
			}
		}
		lineIndex++;
	}

	cout<< endl;
#pragma endregion

	float ampStep = 0.03;
	float freqStep = 0.005;
	freq -= freqStep;
	for(int tempStep = 0 ; tempStep < 60 ; tempStep++)
	{
	//int time = 20;
		//amp = ampStep * tempStep;
		amp = ampStep * 40;
		freq += freqStep;
	//create density with or without perlin noise
	Perlin perlin(octave, freq, amp, seed);
	for(int i = 0 ; i < objectCount ; i++)
	{
		center[0] = centerArray[i * 3 + 0];
		center[1] = centerArray[i * 3 + 1];
		center[2] = centerArray[i * 3 + 2];

		createDensity(typeArray[i], doTriFilter, XYZC, DELT, coordOrigin, center, radiusArray[i], densityArray, perlin);
	}



	//set BMP outout
	unsigned int width = resolution[0];
	unsigned int height = resolution[1];
	BMP output;
	output.SetSize(width, height);
	output.SetBitDepth(24);

	//Do ray march of each pixel
	for(int py = 0 ; py < resolution[1] ; ++py)
	{
		if(py % 20 == 0)
			cout << "Computing y: " << py << endl; 
		for(int px = 0 ; px < resolution[0] ; ++px)
		{
			float sceneDis = sqrt(vDir[0] * vDir[0] + vDir[1] * vDir[1] + vDir[2] * vDir[2]);

			//calculate ray direction
			float rayDir[3];
			rayDir[0] = vDir[0] + sceneDis * tan(FOVY / 180 * M_PI) * resolution[0] / resolution[1] * (-1 + (1 + px * 2.0) /resolution[0]);
			rayDir[1] = vDir[1] + sceneDis * tan(FOVY / 180 * M_PI) * (1 - (1 + py * 2.0) /resolution[1]);	
			rayDir[2] = -1 * sceneDis;

			//normalize ray direction
			float tempLength = sqrt(rayDir[0] * rayDir[0] + rayDir[1] * rayDir[1] + rayDir[2] * rayDir[2]);
			rayDir[0] = rayDir[0] / tempLength;
			rayDir[1] = rayDir[1] / tempLength;
			rayDir[2] = rayDir[2] / tempLength;


			float colorAccumR = 0;
			float colorAccumG = 0;
			float colorAccumB = 0;
			float accumDensity = 0;
			float transmittance = 1;
			float rayPos[3];

			//set voxel buffer 8 vertices coordinate
			float corner1[3] = {coordOrigin[0],					 coordOrigin[1],				  coordOrigin[2]};//front bottom left
			float corner2[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1],				  coordOrigin[2]};//front bottom right
			float corner3[3] = {coordOrigin[0],					 coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2]};//front top left
			float corner4[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2]};//front top right
			float corner5[3] = {coordOrigin[0],					 coordOrigin[1],				  coordOrigin[2] - XYZC[2] * DELT};//back bottom left
			float corner6[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1],				  coordOrigin[2] - XYZC[2] * DELT};//back bottom right
			float corner7[3] = {coordOrigin[0],					 coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2] - XYZC[2] * DELT};//back top left
			float corner8[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2] - XYZC[2] * DELT};//back top right

			//calculate distance from eye to the 8 vertices
			float distance[8];
			distance[0] = calDistance(eyePosition, corner1);
			distance[1] = calDistance(eyePosition, corner2);
			distance[2] = calDistance(eyePosition, corner3);
			distance[3] = calDistance(eyePosition, corner4);
			distance[4] = calDistance(eyePosition, corner5);
			distance[5] = calDistance(eyePosition, corner6);
			distance[6] = calDistance(eyePosition, corner7);
			distance[7] = calDistance(eyePosition, corner8);

			int farestIndex = compareLargest(sizeof(distance) / sizeof(distance[0]), distance);
			int nearestIndex = compareSmallest(sizeof(distance) / sizeof(distance[0]), distance);
			for(int rayStep = distance[nearestIndex] / STEP ; rayStep < distance[farestIndex] / STEP + 1; rayStep++)
			{
				rayPos[0] = eyePosition[0] + rayDir[0] * STEP * rayStep;
				rayPos[1] = eyePosition[1] + rayDir[1] * STEP * rayStep;
				rayPos[2] = eyePosition[2] + rayDir[2] * STEP * rayStep;
				if(rayPos[2] < coordOrigin[2])
				{
					//if current ray position is inside the voxel buffer
					if(rayPos[0] > coordOrigin[0]  && rayPos[0] < coordOrigin[0] + XYZC[0] * DELT && rayPos[1] > coordOrigin[1] && rayPos[1] < coordOrigin[1] + XYZC[1] * DELT && rayPos[2] < coordOrigin[2] && rayPos[2] > coordOrigin[2] - XYZC[2] * DELT)
					{
						int indexX = abs((int)(rayPos[0] - coordOrigin[0])); //truncate to integer
						int indexY = abs((int)(rayPos[1] - coordOrigin[1]));
						int indexZ = abs((int)(coordOrigin[2] - rayPos[2] ));

						int arrayIndex = indexX + XYZC[0] * indexY + XYZC[0] * XYZC[1] * indexZ;

						float density = 0;

						//choosing if do tri-linear interpolation
						if(doInterpolation == false)
							density = densityArray[arrayIndex];
						else
							density = doTriLinear( rayPos, densityArray, density, arrayIndex, XYZC, DELT, coordOrigin);


						accumDensity = accumDensity + density;
						transmittance = exp(-k * accumDensity);

	

						//build multi light sources	
						float tempValueR = 0, tempValueG = 0, tempValueB = 0;
						int lightSourceNum = sizeof(arrLightPos) / sizeof(arrLightPos[0]);
						for(int n = 0 ; n <lightSourceNum ; n++)
						{
							float lightValue = createLightSource(arrLightPos[n], arrLightArray[n], arrayIndex, coordOrigin, DELT, STEP, k, XYZC, densityArray, doInterpolation);
							tempValueR += lightValue * *(arrLightColor[n] + 0);
							tempValueG += lightValue * *(arrLightColor[n] + 1);
							tempValueB += lightValue * *(arrLightColor[n] + 2);
						}


						float colorSingleR;
						float colorSingleG;
						float colorSingleB;
						//Judge if create image with multi material color
						if(doMultiMaterialColor)
						{
							bool insideRadiusFlag = false;
							for(int n = 0 ; n < objectCount ; n++)
							{
								if(rayPos[0] > centerArray[n * 3 + 0] - radiusArray[n] && rayPos[0] < centerArray[n * 3 + 0] + radiusArray[n] && 
									rayPos[1] > centerArray[n * 3 + 1] - radiusArray[n] && rayPos[1] < centerArray[n * 3 + 1] + radiusArray[n] && 
									rayPos[2] > -centerArray[n * 3 + 2] - radiusArray[n] && rayPos[2] < -centerArray[n * 3 + 2] + radiusArray[n])
								{
									if(n < sizeof(arrMaterialColor) / sizeof(arrMaterialColor[0]))
									{
										colorSingleR = *(arrMaterialColor[n] + 0) * (1 - exp(-k * density)) / k * transmittance * tempValueR * 255;
										colorSingleG = *(arrMaterialColor[n] + 1) * (1 - exp(-k * density)) / k * transmittance * tempValueG * 255;
										colorSingleB = *(arrMaterialColor[n] + 2) * (1 - exp(-k * density)) / k * transmittance * tempValueB * 255;
									}
									else
									{
										colorSingleR = *(arrMaterialColor[0] + 0) * (1 - exp(-k * density)) / k * transmittance * tempValueR * 255;
										colorSingleG = *(arrMaterialColor[0] + 1) * (1 - exp(-k * density)) / k * transmittance * tempValueG * 255;
										colorSingleB = *(arrMaterialColor[0] + 2) * (1 - exp(-k * density)) / k * transmittance * tempValueB * 255;
									}
									insideRadiusFlag = true;
									break;
								}
							}

							if(insideRadiusFlag == false  )
							{
								if(sizeof(arrMaterialColor) / sizeof(arrMaterialColor[0]) == 1)
								{
									colorSingleR = *(arrMaterialColor[1] + 0) * (1 - exp(-k * density)) / k * transmittance * tempValueR * 255;
									colorSingleG = *(arrMaterialColor[1] + 1) * (1 - exp(-k * density)) / k * transmittance * tempValueG * 255;
									colorSingleB = *(arrMaterialColor[1] + 2) * (1 - exp(-k * density)) / k * transmittance * tempValueB * 255;
								}
								else
								{
									colorSingleR = *(arrMaterialColor[0] + 0) * (1 - exp(-k * density)) / k * transmittance * tempValueR * 255;
									colorSingleG = *(arrMaterialColor[0] + 1) * (1 - exp(-k * density)) / k * transmittance * tempValueG * 255;
									colorSingleB = *(arrMaterialColor[0] + 2) * (1 - exp(-k * density)) / k * transmittance * tempValueB * 255;
								}
							}
						}
						else//create image with single material color
						{
							colorSingleR = materialColor[0] * (1 - exp(-k * density)) / k * transmittance * tempValueR * 255;
							colorSingleG = materialColor[1] * (1 - exp(-k * density)) / k * transmittance * tempValueG * 255;
							colorSingleB = materialColor[2] * (1 - exp(-k * density)) / k * transmittance * tempValueB * 255;
						}

						//float colorSingleR = density * materialColor[0] * transmittance * tempValueR * 255;
						//float colorSingleG = density * materialColor[1] * transmittance * tempValueG * 255;
						//float colorSingleB = density * materialColor[2] * transmittance * tempValueB * 255;

						colorAccumR = colorAccumR + colorSingleR;
						colorAccumG = colorAccumG + colorSingleG;
						colorAccumB = colorAccumB + colorSingleB;
					}	
				}
			}


			transmittance = exp(-k * accumDensity);
			
			//Add single background color
			//colorAccumR = colorAccumR + bgColor[0] * 255 * transmittance;
			//colorAccumG = colorAccumG + bgColor[1] * 255 * transmittance;
			//colorAccumB = colorAccumB + bgColor[2] * 255 * transmittance;

			//Add gradient background color
			colorAccumR = colorAccumR + (bgColor[0] + (bgColor2[0] - bgColor[0]) / resolution[1] * py) * 255 * transmittance;
			colorAccumG = colorAccumG + (bgColor[1] + (bgColor2[1] - bgColor[1]) / resolution[1] * py) * 255 * transmittance;
			colorAccumB = colorAccumB + (bgColor[2] + (bgColor2[2] - bgColor[2]) / resolution[1] * py) * 255 * transmittance;

			if(colorAccumR > 255)
				colorAccumR = 255;
			if(colorAccumG > 255)
				colorAccumG = 255;
			if(colorAccumB > 255)
				colorAccumB = 255;

			output(px, py)->Red = colorAccumR;
			output(px, py)->Green = colorAccumG;
			output(px, py)->Blue = colorAccumB;

			//Test ray direction
			//output(px, py)->Red = abs(rayDir[0]) * 255;
			//output(px, py)->Green = abs(rayDir[1]) * 255;
			//output(px, py)->Blue = abs(rayDir[2]) * 255;
		}
	}
	stringstream str_oct, str_amp, str_freq, str_seed;
	str_oct << octave;
	str_amp << tempStep;
	str_freq << freq;
	str_seed << seed;

	string newSaveFileName = saveFileName.substr(0,saveFileName.length() - 4);

	string savePath = newSaveFileName + "_Oct_" + str_oct.str() +"_Amp_40" /*+ str_amp.str()*/ + "_Freq_" + str_freq.str() + "_Seed_" + str_seed.str() + ".bmp"; 
	cout << savePath << endl;
	output.WriteToFile(savePath.c_str());

	for(int i =0 ; i < XYZC[0] * XYZC[1] * XYZC[2] ; i++)
	{
		densityArray[i] = 0;
		lightArray[i] = -1;
		lightArray2[i] = -1;
		lightArray3[i] = -1;
	}
	}

	
	return 0;
}

int* findSpace(const char* c)
{
	int index[2] = {0, 0};

	for(int j = 0; j < strlen(c) ; j++)
	{
		if(c[j] == ' ')
		{
			if(index[0] == 0)
				index[0] = j;
			else
				index[1] = j;
		}
	}
	stringstream ss1,ss2;
	ss1<<index[0];
	ss2<<index[1];

	cout << "index[0] = " << ss1.str() << " ; index[1] = " << ss2.str() << endl;
	return index;
}

float doTriLinear(float* rayPos, float* densityArray, float density, int arrayIndex, int* XYZC, float DELT, float* coordOrigin)
{
	int indexZ = arrayIndex / (XYZC[0] * XYZC[1]);
	int indexY = (arrayIndex - indexZ * XYZC[0] * XYZC[1]) / XYZC[0];
	int indexX = arrayIndex - indexZ * XYZC[0] * XYZC[1] - indexY * XYZC[0];

	if(densityArray[arrayIndex] != 0)
	{
		for(int z = 0 ; z < 2 ; z++)
		{
			for(int y = 0 ; y < 2 ; y++)
			{
				for(int x = 0 ; x < 2 ; x++)
				{
					bool ifOutside = false;

					float wight_X;
					float wight_Y;
					float wight_Z;

					if(indexX == XYZC[0] - 1)
					{
						if(x == 0)
							wight_X = 1;
						else
						{
							wight_X = 0;
							ifOutside = true;
						}
					}
					else
					{
						wight_X = (DELT - abs(rayPos[0] - (coordOrigin[0] + (x + indexX)) * DELT)) / DELT;
					}

					if(indexY == XYZC[1] - 1)
					{
						if(y == 0)
							wight_Y = 1;
						else
						{
							wight_Y = 0;
							ifOutside = true;										
						}
					}
					else
					{
						wight_Y = (DELT - abs(rayPos[1] - (coordOrigin[1] + (y + indexY)) * DELT)) / DELT;
					}

					if(indexZ == XYZC[2] - 1)
					{
						if(z == 0)
							wight_Z = 1;
						else
						{
							wight_Z = 0;
							ifOutside = true;										
						}
					}
					else
					{
						wight_Z = (DELT - abs(rayPos[2] - (coordOrigin[2] - (z + indexZ)) * DELT)) / DELT;
					}
					float wight_XYZ = wight_X * wight_Y * wight_Z;

					int densityIndex = arrayIndex + x + y * XYZC[0] + z * XYZC[0] * XYZC[1]; 

										//if(densityIndex < XYZC[0] * XYZC[1] * XYZC[2])
					if(ifOutside == false)
						density += wight_XYZ * densityArray[densityIndex];
				}
			}
		}
	}
	else
		density = densityArray[arrayIndex];

	return density;
							//////////////////////////////////
}

float createLightSource(float* lightPos, float* lightArray, int arrayIndex, float* coordOrigin, float DELT, float STEP, float k, int* XYZC, float* densityArray, bool doInterpolation)
{
	float lightValue;
	if(lightArray[arrayIndex] == -1)
	{
		int indexZ = arrayIndex / (XYZC[0] * XYZC[1]);
		int indexY = (arrayIndex - indexZ * XYZC[0] * XYZC[1]) / XYZC[0];
		int indexX = arrayIndex - indexZ * XYZC[0] * XYZC[1] - indexY * XYZC[0];

		//calculate light direction
		float lightDir[3];
		lightDir[0] = coordOrigin[0] + DELT * (indexX + 0.5) - lightPos[0];
		lightDir[1] = coordOrigin[1] + DELT * (indexY + 0.5) - lightPos[1];
		lightDir[2] = coordOrigin[2] - DELT * (indexZ + 0.5) - lightPos[2];

		//normalize light direction
		float lightLength = sqrt(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
		lightDir[0] = lightDir[0] / lightLength;//Normalize light direction
		lightDir[1] = lightDir[1] / lightLength;
		lightDir[2] = lightDir[2] / lightLength;
	
		float lightRayPos[3];
		float accumDensity = 0;
		
		//set voxel buffer 8 vertices coordinate
		float corner1[3] = {coordOrigin[0],					 coordOrigin[1],				  coordOrigin[2]};//front bottom left
		float corner2[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1],				  coordOrigin[2]};//front bottom right
		float corner3[3] = {coordOrigin[0],					 coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2]};//front top left
		float corner4[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2]};//front top right
		float corner5[3] = {coordOrigin[0],					 coordOrigin[1],				  coordOrigin[2] - XYZC[2] * DELT};//back bottom left
		float corner6[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1],				  coordOrigin[2] - XYZC[2] * DELT};//back bottom right
		float corner7[3] = {coordOrigin[0],					 coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2] - XYZC[2] * DELT};//back top left
		float corner8[3] = {coordOrigin[0] + XYZC[0] * DELT, coordOrigin[1] + XYZC[1] * DELT, coordOrigin[2] - XYZC[2] * DELT};//back top right
		
		//calculate distance from eye to the 8 vertices
		float distance[8];
		distance[0] = calDistance(lightPos, corner1);
		distance[1] = calDistance(lightPos, corner2);
		distance[2] = calDistance(lightPos, corner3);
		distance[3] = calDistance(lightPos, corner4);
		distance[4] = calDistance(lightPos, corner5);
		distance[5] = calDistance(lightPos, corner6);
		distance[6] = calDistance(lightPos, corner7);
		distance[7] = calDistance(lightPos, corner8);


		int farestIndex = compareLargest(sizeof(distance) / sizeof(distance[0]), distance);
		int nearestIndex = compareSmallest(sizeof(distance) / sizeof(distance[0]), distance);		
		for(int lightStep = distance[nearestIndex] / STEP ; lightStep < distance[farestIndex] / STEP + 1; lightStep++)
		{
			lightRayPos[0] = lightPos[0] + lightDir[0] * STEP * lightStep;
			lightRayPos[1] = lightPos[1] + lightDir[1] * STEP * lightStep;
			lightRayPos[2] = lightPos[2] + lightDir[2] * STEP * lightStep;

			if(lightRayPos[0] >  coordOrigin[0] && lightRayPos[0] <  coordOrigin[0] + XYZC[0] * DELT && lightRayPos[1] >  coordOrigin[1] && lightRayPos[1] <  coordOrigin[1] + XYZC[1] * DELT && lightRayPos[2] <  coordOrigin[2] && lightRayPos[2] >  coordOrigin[2] - XYZC[2] * DELT)
			{
				int lightIndexX = (int)(lightRayPos[0] - coordOrigin[0]); //truncate to integer
				int lightIndexY = (int)(lightRayPos[1] - coordOrigin[1]);
				int lightindexZ = abs((int)(coordOrigin[2] - lightRayPos[2]));

				int arrayIndex2 = lightIndexX + XYZC[0] * lightIndexY + XYZC[0] * XYZC[1] * lightindexZ;
									
				float density = 0;
				if(doInterpolation == false)
					density = densityArray[arrayIndex2];
				else/////////add Tri-linear Interpolation
					density = doTriLinear(lightRayPos, densityArray, density, arrayIndex2, XYZC, DELT, coordOrigin);

				accumDensity += density;

				if(arrayIndex2 == arrayIndex)
					break;
			}
		}


		lightValue = exp(-k * accumDensity);
		lightArray[arrayIndex] = lightValue;
	}
	else
		lightValue = lightArray[arrayIndex];

	return lightValue;
}

float calDistance(float* p1, float* p2)
{
	float length;
	length = sqrt( pow(p1[0] - p2[0],2) + pow(p1[1] - p2[1],2) + pow(p1[2] - p2[2],2));
	return length;
}

int compareLargest(int size, float* m)
{
	int index = 0;

	for(int i = 1 ; i < size ; i++)
	{
		if(m[index] < m[i])
		{index = i;}
	}

	return index;
}

int compareSmallest(int size, float* m)
{
	int index = 0;

	for(int i = 1 ; i < size ; i++)
	{
		if(m[index] > m[i])
		{index = i;}
	}

	return index;
}

void createDensity(int type,bool doTriFilter, int XYZC[], float DELT, float coordOrigin[], float center[], float radius, float densityArray[], Perlin p)
{
	for(int z =0 ; z < XYZC[2] ; z++)
	{
		for(int y =0 ; y < XYZC[1] ; y++)
		{
			for(int x = 0 ; x < XYZC[0] ; x++)
			{
				float disX, disY, disZ;
				disX = (coordOrigin[0] + (0.5 + x) * DELT) - center[0];
				disY = (coordOrigin[1] + (0.5 + y) * DELT) - center[1];
				disZ = (coordOrigin[2] + (0.5 + z) * DELT) - center[2];


				float distance = sqrt(disX * disX + disY * disY + disZ * disZ);
				
				float density;

				if(type ==0)//sphere
					density = 1- distance / radius;
				else if(type == 1)//sphere cloud
				{
					density = p.Get(disX,disY,disZ) + (1- distance / radius);
				}
				else if (type == 2)//Pyroclastics
					density =max((radius - distance / radius + abs(p.Get(disX,disY,disZ))),0);
				else if (type == 3 || type == 4)//quad / quad cloud
				{
					int xValue, yValue, zValue;
		
					if(x >= XYZC[0] / 2)
						xValue = XYZC[0] - 1 - x;
					else
						xValue = x;

					if(y >= XYZC[1] / 2)
						yValue = XYZC[1] - 1 - y;
					else
						yValue = y;

					if(z >= XYZC[2] / 2)
						zValue = XYZC[2] - 1 - z;
					else
						zValue = z;
					if(type == 3)//quad
					{
						if(xValue < yValue)
						{
							if(xValue < zValue)
								density = 1- abs(disX) / radius;
							else
								density = 1- abs(disZ) / radius;
						}
						else 
						{
							if(yValue < zValue)
								density = 1- abs(disY) / radius;
							else
								density = 1- abs(disZ) / radius;
						}
					}
					else if(type == 4)//quad cloud
					{
						if(xValue < yValue)
						{
							if(xValue < zValue)
								density =p.Get(disX,disY,disZ) + 1- abs(disX) / radius;
							else
								density =p.Get(disX,disY,disZ) +  1- abs(disZ) / radius;
						}
						else 
						{
							if(yValue < zValue)
								density =p.Get(disX,disY,disZ) + 1- abs(disY) / radius;
							else
								density =p.Get(disX,disY,disZ) + 1- abs(disZ) / radius;
						}
					}
				}

				if(density < 0)
					density = 0;
				else if(density > 1)
					density = 1;

				//if do the Tri-Filter density splat
				//it will splat out the current density to the 6 nearest voxels and 12 second nearest voxels. 
				//According to the distance from the original voxel, the neighbor voxels would get different density. 
				if(doTriFilter)
				{
					density = density * 0.17;
					
					float density2, density3;

					density2 = density * (1.0 - 1.0 / sqrt(3.0));
					density3 = density * (1.0 - sqrt(2.0) / sqrt(3.0));

					for(int i = -1 ; i < 2 ; i++)
					{
						for(int j = -1 ; j < 2 ; j++)
						{
							for(int k = -1 ; k < 2 ; k++)
							{
								if((x + i) >= 0 && (y + j) >= 0 && (z + k) >= 0 && (x + i) < XYZC[0] && (y + j) < XYZC[1] && (z + k) < XYZC[2])
								{
									if( i == 0 && j == 0 && k == 0)//original voxel
									{
										continue;
									}
									else if((i == 0 && j ==0) || (i == 0 && k == 0) || (j == 0 && k ==0))//nearest voxel
									{
										densityArray[(x + i) + XYZC[0] * (y + j) + XYZC[0] * XYZC[1] * (z + k)] += density2;
									}
									else if(i == 0 || j == 0 || k == 0)//second nearest voxel
									{
										densityArray[(x + i) + XYZC[0] * (y + j) + XYZC[0] * XYZC[1] * (z + k)] += density3;
									}
									if(densityArray[(x + i) + XYZC[0] * (y + j) + XYZC[0] * XYZC[1] * (z + k)] > 1)
										densityArray[(x + i) + XYZC[0] * (y + j) + XYZC[0] * XYZC[1] * (z + k)] = 1;

								}
							}
						}
					}
				}


				densityArray[x +  XYZC[0] * y +  XYZC[0] * XYZC[1] * z] += density;
				if(densityArray[x + XYZC[0] * y + XYZC[0] * XYZC[1] * z] > 1)
					densityArray[x + XYZC[0] * y + XYZC[0] * XYZC[1] * z] = 1;



			}
		}
	}
}

string WstringToString(const wstring str)
{
    unsigned len = str.size() * 4;
    setlocale(LC_CTYPE, "");
    char *p = new char[len];
    wcstombs(p,str.c_str(),len);
    std::string str1(p);
    delete[] p;
    return str1;
}

wstring StringToWstring(const string str)
{
    unsigned len = str.size() * 2;
    setlocale(LC_CTYPE, "");     
    wchar_t *p = new wchar_t[len];
    mbstowcs(p,str.c_str(),len);
    std::wstring str1(p);
    delete[] p;
    return str1;
}

int compare(int x, int y, int z)
{
	if(x < y)
	{
		if(x < z)
			return 1;
		else
			return 3;
	}
	else 
	{
		if(y < z)
			return 2;
		else
			return 3;
	}
}