#ifndef Point_h
#define Point_h

class Point
{
public:
	Point() { }
	Point(const Vec3 &p) : m_position(p) {}

	Vec3 GetPosition() const { return m_position; }

	void SetPosition(const Vec3 &p) { m_position = p; }

private:
	Vec3 m_position;
};

#endif
