//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_MATH_H
#define TANK_TROUBLE_MATH_H
#include <vector>
#include <utility>
#include "Vec.h"

namespace TankTrouble::util
{
    //弧度角度转换
    double rad2Deg(double rad);

    double deg2Rad(double deg);

    //根据向量计算旋转角
    double vector2Angle(const util::Vec& v);

    Vec polar2Cart(double theta, double p, Vec O = Vec(0, 0));

    //已知矩形中心点坐标，旋转角，宽高，求四个顶点坐标
    std::vector<Vec> getCornerVec(const Vec& pos, double angle, int w, int h);

    //提供矩形两个检测轴的单位向量，矩形中心，圆心，矩形宽高，半径判断矩形和圆形是否碰撞(重叠)
    bool checkRectCircleCollision(const Vec& vec1, const Vec& vec2,
                                  const Vec& rectCenter, const Vec& circleCenter,
                                  int width, int height, double r);

    //已知直线两点求直线的一般式
    void twoPointToGeneral(Vec p1, Vec p2, double* A, double* B, double* C);

    //一般式求两直线交点
    bool intersectionOfLines(double A1, double B1, double C1, double A2, double B2, double C2, Vec* p);

    //求两个线段交点
    bool intersectionOfSegments(Vec p1, Vec p2, Vec p3, Vec p4, Vec* i);

    //求一个角度关于X轴对称的角度
    double angleFlipX(double angle);

    //求一个角度关于y轴对称的角度
    double angleFlipY(double angle);

    //根据矩形旋转角得到其两条检测轴的单位向量
    std::pair<Vec, Vec> getUnitVectors(double angleDeg);

    util::Vec getUnitVector(double angleDeg);

    //判断两个矩形是否碰撞(angle为旋转角)
    bool checkRectRectCollision(double angle1, Vec center1, double W1, double H1,
                                double angle2, Vec center2, double W2, double H2);

    //两点直线距离
    double distanceOfTwoPoints(const Vec& p1, const Vec& p2);

    //求两向量夹角(度)
    double angleBetweenVectors(const Vec& v1, const Vec& v2);

    int getRandomNumber(int low, int high);
}

#endif //TANK_TROUBLE_MATH_H
