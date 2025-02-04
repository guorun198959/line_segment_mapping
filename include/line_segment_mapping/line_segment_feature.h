#ifndef LINE_SEGMENT_MAPPING_LINE_SEGMENT_FEATURE_H
#define LINE_SEGMENT_MAPPING_LINE_SEGMENT_FEATURE_H

#include <memory>
#include <unordered_map>
#include "open_karto/Karto.h"

namespace karto
{
inline double point_to_line_distance(double A, double B, double C, double x, double y)
{
  return fabs(A * x + B * y + C) / sqrt(A * A + B * B);
}

class LineSegment
{
public:
  LineSegment()
  {
  }

  LineSegment(const Vector2<double>& startPoint, const Vector2<double>& endPoint)
  {
    m_StartPoint = startPoint;
    m_EndPoint = endPoint;
    m_Barycenter = (m_StartPoint + m_EndPoint) / 2;
    m_DirectionVector = m_EndPoint - m_StartPoint;
    assert(!std::isnan(m_DirectionVector.GetX()) && !std::isnan(m_DirectionVector.GetY()));  // 排除NaN值的情况

    m_Length = m_DirectionVector.Length();
    assert(m_Length >= KT_TOLERANCE);

    m_Heading = atan2(m_DirectionVector.GetY(), m_DirectionVector.GetX());
    assert(math::InRange(m_Heading, -KT_PI, KT_PI));
    m_UpdateTimes = 1;

    double A = m_EndPoint.GetY() - m_StartPoint.GetY();
    double B = m_StartPoint.GetX() - m_EndPoint.GetX();
    double C = m_EndPoint.GetX() * m_StartPoint.GetY() - m_StartPoint.GetX() * m_EndPoint.GetY();
    m_Role = fabs(C) / sqrt(A * A + B * B);

    double x = -A * C / (A * A + B * B);
    double y = -B * C / (A * A + B * B);
    m_Phi = atan2(y, x);

    m_pScan = NULL;
  }

  virtual ~LineSegment()
  {
    m_pScan = NULL;
  }

public:
  const Vector2<double>& GetStartPoint() const
  {
    return m_StartPoint;
  }

  const Vector2<double>& GetEndPoint() const
  {
    return m_EndPoint;
  }

  const Vector2<double>& GetBarycenter() const
  {
    return m_Barycenter;
  }

  const Vector2<double>& GetDirectionVector() const
  {
    return m_DirectionVector;
  }

  const double& GetHeading() const
  {
    return m_Heading;
  }

  const double& GetLength() const
  {
    return m_Length;
  }

  const int& GetUpdateTimes() const
  {
    return m_UpdateTimes;
  }

  void SetUpdateTimes(int updateTimes)
  {
    m_UpdateTimes = updateTimes;
  }

  const double& GetRole() const
  {
    return m_Role;
  }

  const double& GetPhi() const
  {
    return m_Phi;
  }

  void SetScan(LocalizedRangeScan* pScan)
  {
    m_pScan = pScan;
  }

  LineSegment GetGlobalLineSegments() const
  {
    Pose2 scanPose = m_pScan->GetSensorPose();
    double cosine = cos(scanPose.GetHeading());
    double sine = sin(scanPose.GetHeading());

    Vector2<double> startPoint, endPoint;

    startPoint.SetX(m_StartPoint.GetX() * cosine - m_StartPoint.GetY() * sine + scanPose.GetX());
    startPoint.SetY(m_StartPoint.GetX() * sine + m_StartPoint.GetY() * cosine + scanPose.GetY());
    endPoint.SetX(m_EndPoint.GetX() * cosine - m_EndPoint.GetY() * sine + scanPose.GetX());
    endPoint.SetY(m_EndPoint.GetX() * sine + m_EndPoint.GetY() * cosine + scanPose.GetY());

    LineSegment lineSegment(startPoint, endPoint);
    return lineSegment;
  }

private:
  Vector2<double> m_StartPoint;
  Vector2<double> m_EndPoint;

  double m_Heading;  // 线段方向
  double m_Length;
  Vector2<double> m_Barycenter;       // 线段的中心点
  Vector2<double> m_DirectionVector;  // 单位方向向量

  const LocalizedRangeScan* m_pScan;  // 该线段关联的激光扫描

  int m_UpdateTimes;  // 线段被更新的次数，初始为1
  double m_Role;
  double m_Phi;
};

typedef std::vector<LineSegment> LineSegmentVector;
typedef std::unordered_map<int, LineSegment> LineSegmentHashTable;
typedef std::shared_ptr<LineSegment> LineSegmentPtr;
typedef std::vector<LineSegmentPtr> LineSegmentPtrVector;
typedef std::unordered_map<int, LineSegmentPtrVector> LineSegmentPtrVectorHashTable;

}  // namespace karto

#endif  // LINE_SEGMENT_MAPPING_LINE_SEGMENT_FEATURE_H