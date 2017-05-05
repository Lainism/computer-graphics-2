
#include "AreaLight.hpp"


namespace FW {


void AreaLight::draw(const Mat4f& worldToCamera, const Mat4f& projection) {
    glUseProgram(0);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((float*)&projection);
    glMatrixMode(GL_MODELVIEW);
    Mat4f S = Mat4f::scale(Vec3f(m_size,1));
    Mat4f M = worldToCamera *m_xform * S;
    glLoadMatrixf((float*)&M);
    glBegin(GL_TRIANGLES);
    glColor3fv( &m_E.x );
    glVertex3f(1,1,0); glVertex3f(1,-1,0); glVertex3f( -1,-1,0 );
    glVertex3f(1,1,0); glVertex3f( -1,-1,0 ); glVertex3f(-1,1,0); 
    glEnd();
}

void AreaLight::sample(float& pdf, Vec3f& p, int base, Random& rnd) {
    // YOUR CODE HERE (R1): Integrate your area light implementation.

    // (this does not do what it's supposed to!)
	float x = rnd.getF32(-1, 1) * m_size.x;
	float y = rnd.getF32(-1, 1) * m_size.y;

	// 1 / surface area of light
	pdf = 1.0f / (4 * m_size.x*m_size.y);
	Vec4f temp;
	temp[0] = x;
	temp[1] = y;
	temp[3] = 1.0f;

	p = (m_xform * temp).getXYZ();
}


} // namespace FW
