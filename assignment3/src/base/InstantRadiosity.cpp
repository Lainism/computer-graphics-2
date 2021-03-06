#include "InstantRadiosity.hpp"


namespace FW 
{


void InstantRadiosity::castIndirect(RayTracer *rt, MeshWithColors *scene, const LightSource& ls, int num)
{

    // If the caller requests a different number of lights than before, reallocate everything.
    // (This is OpenGL resource management stuff, don't touch unless you specifically need to)
    if (m_indirectLights.size() != unsigned(num))
    {
        printf("Deleting %i indirect light sources.\n", num);
        for (auto& iter : m_indirectLights)// = m_indirectLights.begin(); iter != m_indirectLights.end(); iter++)
            iter.freeShadowMap();
        m_indirectLights.resize(num);
		for (auto& iter : m_indirectLights)// = m_indirectLights.begin(); iter != m_indirectLights.end(); iter++)
            iter.setEnabled(false);
    }

    // Request #num exiting rays from the light.
    std::vector<Vec3f> origs, dirs, E_times_pdf;
    ls.sampleEmittedRays(num, origs, dirs, E_times_pdf);
    
    // You'll probably want to implement the sampleEmittedRays() function before the rest of this function,
    // because otherwise you won't have much to trace.
    //
    // At this point m_indirectLights holds #num lights that are off.
    //
    // Loop through the rays and fill in the corresponding lights in m_indirectLights
    // based on what happens to the ray.
    for (int i = 0; i < num; i++)
    {
        RaycastResult hit = rt->raycast(origs[i], dirs[i]);

        if ( hit.tri != nullptr )
        {
            // YOUR CODE HERE (R4):
            // Ray hit the scene, now position the light m_indirectLights[i] correctly,
            // color it based on the texture or diffuse color, etc. (see the LightSource declaration for the list 
            // of things that a light source needs to have)
            // A lot of this code is like in the Assignment 2's corresponding routine.

            // Replace this with true once your light is ready to be used in rendering:
            m_indirectLights[i].setEnabled(true);

			// Where is the hit point?
			m_indirectLights[i].setPosition(hit.point);

			// What direction should the light point to?
			Vec3f n = hit.tri->normal();
			if (dot(n, hit.dir) > 0) { n = -n; }
			Mat3f B = formBasis( -n );
			m_indirectLights[i].setOrientation( B );

			// Set the FOV based on the FOV member variable in InstantRadiosity
			m_indirectLights[i].setFOV(m_indirectFOV);

			// What is the emission? It is the entry in the emission intensity list returned by the
			// sampler, multiplied by the surface color{ so if there's a texture, you'll need to fetch
			// the value.Otherwise use the surface diuse color.

			float alpha = hit.u;
			float beta = hit.v;

			Vec3f Ei = E_times_pdf[i];

			// check for texture
			const auto mat = hit.tri->m_material;
			if (mat->textures[MeshBase::TextureType_Diffuse].exists())
			{
				const Texture& tex = mat->textures[MeshBase::TextureType_Diffuse];
				const Image& teximg = *tex.getImage();

				Vec2f uv;
				uv[0] = alpha;
				uv[1] = beta;
				Vec2i texelCoords = getTexelCoords(uv, teximg.getSize());

				auto diffuse = teximg.getVec4f(texelCoords).getXYZ();
				Ei *= diffuse;
			}
			else
			{
				// no texture, use constant albedo from material structure.

				// (this is just one line)
				Ei *= mat->diffuse.getXYZ();
			}

			m_indirectLights[i].setEmission(Ei);
        }
        else
        {
            // If we missed the scene, disable the light so it's skipped in all rendering operations.
            m_indirectLights[i].setEnabled(false);
        }
    }
}

void InstantRadiosity::renderShadowMaps(MeshWithColors *scene)
{
    // YOUR CODE HERE (R4):
    // Loop through all lights, and call the shadow map renderer for those that are enabled.
    // (see App::renderFrame for an example usage of the shadow map rendering call)
	int light_num = this->getNumLights();

	for (int i = 0; i < light_num; i++) {
		if (m_indirectLights[i].isEnabled())
		{
			m_indirectLights[i].renderShadowMap(m_gl, scene, &m_smContext);
		}
	}
}


//////////// Stuff you probably will not need to touch:

void InstantRadiosity::setup(GLContext* gl, Vec2i resolution)
{
    m_gl = gl;

    // Clear any existing reserved textures
    for (auto iter = m_indirectLights.begin(); iter != m_indirectLights.end(); iter++)
        iter->freeShadowMap();

    // Set up the shadow map buffers
    m_smContext.setup(resolution);

}


void InstantRadiosity::draw( const Mat4f& worldToCamera, const Mat4f& projection  )
{
    // Just visualize all the light source positions
    for (auto iter = m_indirectLights.begin(); iter != m_indirectLights.end(); iter++) {
        if (iter->isEnabled()) {
            iter->draw(worldToCamera, projection, true, false);
        }
    }
}

GLContext::Program* InstantRadiosity::getShader()
{
    return m_gl->getProgram("MeshBase::draw_generic");
}


}

