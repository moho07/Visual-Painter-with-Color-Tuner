#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

Mat frame;
Mat canvas;
int x2 = 0, y2 = 0;

//hmin, hmax, smin, smax, vmin, vmax
vector<vector<int>> colorpick{ {72, 127, 97, 255, 87, 255}, //Blue
                                {0, 26, 13, 255, 0, 255}, //Orange
                                  {0, 179, 44, 255, 0, 65} }; //Black

vector<Scalar> colorpickdisplay{ {255, 0, 0}, //Blue(BGR)
                                    {0, 165, 255}, //Orange(BGR)
                                        {0, 0, 0} }; //Black(BGR)


Point getContours(Mat mask)
{
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    float area, peri;

    findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<vector<Point>> conpoly(contours.size());
    vector<Rect> boundRect(contours.size());

    Point mypoint(0, 0);

    for (int i = 0; i < contours.size(); i++)
    {
        area = contourArea(contours[i]);

        if (area > 5000)
        {
            peri = arcLength(contours[i], true);
            approxPolyDP(contours[i], conpoly[i], 0.02 * peri, true);

            boundRect[i] = boundingRect(conpoly[i]);

            mypoint.x = boundRect[i].x;
            mypoint.y = boundRect[i].y + boundRect[i].width / 2;

            drawContours(frame, conpoly, i, Scalar(255, 0, 255), 5);
            rectangle(frame, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
        }

        if (boundRect[i].area() > 200000)
            throw 1;

    }

    return mypoint;

}

void findcolor()
{
    bool there = false;

    Mat frameHSV, mask;
    cvtColor(frame, frameHSV, COLOR_BGR2HSV);

    Point mypoint(0, 0);

    for (int i = 0; i < colorpick.size(); i++)
    {
        Scalar lower(colorpick[i][0], colorpick[i][2], colorpick[i][4]);
        Scalar upper(colorpick[i][1], colorpick[i][3], colorpick[i][5]);

        inRange(frameHSV, lower, upper, mask);

        try
        {
            mypoint = getContours(mask);
        }
        catch (int e)
        {
            if (i == colorpick.size() - 1)
                canvas = Mat(Size(frame.cols, frame.rows), CV_8UC3, Scalar(0, 0, 0));
            x2 = 0;
            y2 = 0;
            continue;
        }
        if (mypoint.x != 0 && mypoint.y != 0)
        {
            there = true;
            if (x2 == 0 && y2 == 0)
            {
                x2 = mypoint.x;
                y2 = mypoint.y;
            }
            else
                line(canvas, Point(x2, y2), Point(mypoint.x, mypoint.y), colorpickdisplay[i], 5);

            x2 = mypoint.x;
            y2 = mypoint.y;
        }
    }

    if (!there)
    {
        x2 = 0;
        y2 = 0;
    }
}

void ColorTuner()
{
    VideoCapture webcam(0);

    Mat img, mask, imgHSV;
    int hmin = 0, smin = 0, vmin = 0;
    int hmax = 179, smax = 255, vmax = 255;

    namedWindow("Trackbars", (640, 200)); // Create Window
    createTrackbar("Hue Min", "Trackbars", &hmin, 179);
    createTrackbar("Hue Max", "Trackbars", &hmax, 179);
    createTrackbar("Sat Min", "Trackbars", &smin, 255);
    createTrackbar("Sat Max", "Trackbars", &smax, 255);
    createTrackbar("Val Min", "Trackbars", &vmin, 255);
    createTrackbar("Val Max", "Trackbars", &vmax, 255);

    while (getWindowProperty("Trackbars", WND_PROP_VISIBLE))
    {
        webcam.read(img);
        cvtColor(img, imgHSV, COLOR_BGR2HSV);

        Scalar lower(hmin, smin, vmin);
        Scalar upper(hmax, smax, vmax);

        inRange(imgHSV, lower, upper, mask);

        imshow("Image", img);
        imshow("Mask", mask);
        waitKey(1);
    }

    destroyAllWindows();
}

void VisualPainter()
{
    VideoCapture webcam(0);

    webcam.read(frame);
    imshow("Visual Painter", frame);

    canvas = Mat(Size(frame.cols, frame.rows), CV_8UC3, Scalar(0, 0, 0));

    while (getWindowProperty("Visual Painter", WND_PROP_VISIBLE))
    {
        webcam.read(frame);

        findcolor();

        add(frame, canvas, frame);

        imshow("Visual Painter", frame);

        waitKey(1);
    }

    destroyAllWindows();

}

int main()
{
    int choice;

    do
    {
        cout << "1. Color Tuning\n" << "2. Visual Painter\n" << "3. Exit\n";
        cin >> choice;

        switch (choice)
        {
        case 1: ColorTuner(); break;
        case 2: VisualPainter(); break;
        case 3: exit(0);
        default: cout << "Invalid Choice...Try Again\n"; break;
        }

        cout << "\n";
    } while (choice != 3);

    return 0;
}