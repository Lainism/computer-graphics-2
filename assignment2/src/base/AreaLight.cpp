
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
    // YOUR CODE HERE (R2):
    // You should draw a random point on the light source and evaluate the PDF.
    // Store the results in "pdf" and "p".
    // 
    // Note: The "size" member is _one half_ the diagonal of the light source.
    // That is, when you map the square [-1,1]^2 through the scaling matrix
    // 
    // S = ( size.x    0   )
    //     (   0    size.y )
    // 
    // the result is the desired light source quad (see draw() function above).
    // This means the total area of the light source is 4*size.x*size.y.
    // This has implications for the computation of the PDF.

    // For extra credit, implement QMC sampling using some suitable sequence.
    // Use the "base" input for controlling the progression of the sequence from
    // the outside. If you only implement purely random sampling, "base" is not required.

    // (this does not do what it's supposed to!)

	//source_area = 4*size.x*size.y;
	// x*2+y^2 < 1

	/*
	//Naive way

	int success = 0;
	int n = 10;

	for (int i = 0; i < n; i++) {
		float x = rnd.getF32() * 2 * m_size.x;
		float y = rnd.getF32() * 2 * m_size.y;
		if (x*x + y*y < 1) { success++; }
	}

	pdf = 4 * success / n;
	*/

	// Vec3f win =


    // p = ????????
	// Maybe pick a random point here?
	// Or maybe m_size.x and m_size.y should be random and stored here?
	// Yep, let's try that.

	// Assuming rnd.getF32() gives me float between 0 and 1? Just guessing.
	float x = rnd.getF32() * 2 * m_size.x;
	float y = rnd.getF32() * 2 * m_size.y;

	// Multiplication by two done already in previous step...
	pdf = sqrt(1.0f - x*x - y*y);
    p = Vec4f(m_xform.getCol(3)).getXYZ();
	p[0] = x;
	p[1] = y;
}


} // namespace FW
