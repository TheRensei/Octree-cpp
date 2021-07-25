#ifndef Vec3_h
#define Vec3_h

class Vec3
{
public:
	Vec3() : m_x(0), m_y(0), m_z(0) {}
	explicit Vec3(float x) : m_x(x), m_y(x), m_z(x) {}
	Vec3(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}

	Vec3 operator+(const Vec3 &rOther) const { return Vec3(m_x + rOther.m_x, m_y + rOther.m_y, m_z + rOther.m_z); }
	Vec3 operator+(const float &rOther) const { return Vec3(m_x + rOther, m_y + rOther, m_z + rOther); }

	Vec3 operator-(const Vec3 &rOther) const { return Vec3(m_x - rOther.m_x, m_y - rOther.m_y, m_z - rOther.m_z); }
	Vec3 operator-(const float &rOther) const { return Vec3(m_x - rOther, m_y - rOther, m_z - rOther); }

	float x() const { return m_x; }
	float y() const { return m_y; }
	float z() const { return m_z; }

private:
	float m_x, m_y, m_z;
};

#endif

