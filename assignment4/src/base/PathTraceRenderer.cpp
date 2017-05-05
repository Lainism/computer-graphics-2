#include "PathTraceRenderer.hpp"
#include "RayTracer.hpp"
#include "AreaLight.hpp"

#include <atomic>
#include <chrono>
#include <algorithm>
#include <string>
#include <math.h>



namespace FW {

	bool PathTraceRenderer::m_normalMapped = false;
	bool PathTraceRenderer::debugVis = false;

	void PathTraceRenderer::getTextureParameters(const RaycastResult& hit, Vec3f& diffuse, Vec3f& n, Vec3f& specular)
	{
		//MeshBase::Material* mat = hit.tri->m_material;
		// YOUR CODE HERE (R1)
		// Read value from albedo texture into diffuse.
	    // If textured, use the texture; if not, use Material.diffuse.
	    // Note: You can probably reuse parts of the radiosity assignment.
		float alpha = hit.u;
		float beta = hit.v;

		Vec3f Ei = specular;

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

			// TODO: Raise to the power of 2.2???
			diffuse = Math::pow(teximg.getVec4f(texelCoords).getXYZ(), 2.2f);
			Ei *= diffuse;
		}
		else
		{
			// no texture, use constant albedo from material structure.

			// (this is just one line)
			Ei *= mat->diffuse.getXYZ();
		}
	}


PathTracerContext::PathTracerContext()
    : m_bForceExit(false),
      m_bResidual(false),
      m_scene(nullptr),
      m_rt(nullptr),
      m_light(nullptr),
      m_pass(0),
      m_bounces(0),
      m_destImage(0),
      m_camera(nullptr)
{
}

PathTracerContext::~PathTracerContext()
{
}


PathTraceRenderer::PathTraceRenderer()
{
    m_raysPerSecond = 0.0f;
}

PathTraceRenderer::~PathTraceRenderer()
{
    stop();
}

// This function traces a single path and returns the resulting color value that will get rendered on the image. 
// Filling in the blanks here is all you need to do this time around.
Vec3f PathTraceRenderer::tracePath(float image_x, float image_y, PathTracerContext& ctx, int samplerBase, Random& R, std::vector<PathVisualizationNode>& visualization)
{
	const MeshWithColors* scene = ctx.m_scene;
	RayTracer* rt = ctx.m_rt;
	Image* image = ctx.m_image.get();
	const CameraControls& cameraCtrl = *ctx.m_camera;
	AreaLight* light = ctx.m_light;

	// make sure we're on CPU
	//image->getMutablePtr();

	// get camera orientation and projection
	Mat4f worldToCamera = cameraCtrl.getWorldToCamera();
	Mat4f projection = Mat4f::fitToView(Vec2f(-1, -1), Vec2f(2, 2), image->getSize())*cameraCtrl.getCameraToClip();

	// inverse projection from clip space to world space
	Mat4f invP = (projection * worldToCamera).inverted();


	// Simple ray generation code, you can use this if you want to.

	// Generate a ray through the pixel.
	float x = (float)image_x / image->getSize().x *  2.0f - 1.0f;
	float y = (float)image_y / image->getSize().y * -2.0f + 1.0f;

	// point on front plane in homogeneous coordinates
	Vec4f P0(x, y, 0.0f, 1.0f);
	// point on back plane in homogeneous coordinates
	Vec4f P1(x, y, 1.0f, 1.0f);

	// apply inverse projection, divide by w to get object-space points
	Vec4f Roh = (invP * P0);
	Vec3f Ro = (Roh * (1.0f / Roh.w)).getXYZ();
	Vec4f Rdh = (invP * P1);
	Vec3f Rd = (Rdh * (1.0f / Rdh.w)).getXYZ();

	// Subtract front plane point from back plane point,
	// yields ray direction.
	// NOTE that it's not normalized; the direction Rd is defined
	// so that the segment to be traced is [Ro, Ro+Rd], i.e.,
	// intersections that come _after_ the point Ro+Rd are to be discarded.
	Rd = Rd - Ro;

	// trace!
	RaycastResult result = rt->raycast(Ro, Rd);
	const RTTriangle* pHit = result.tri;

	// if we hit something, fetch a color and insert into image
	Vec3f Ei;
	Vec3f throughput(1, 1, 1);
	float p = 1.0f;

	const float m_pi = 3.1415926535897932384626433832795f;
	Mat3f B = formBasis(n);

	if (result.tri != nullptr)
	{
		// YOUR CODE HERE (R2-R4):
		// Implement path tracing with direct light and shadows, scattering and Russian roulette.
		// Generate ray direction in NDC or camera eye space
		// Transform it to world space
		//  you may need to randomize the position inside the pixel as well in order to perform antialiasing

		// Wasn't this already done above...?

		Vec3f light = 0.0f;
		float prob = 0.0f;
		int bounce = 0;

		// Insert path tracing loop here
		while (true) {
			Vec3f coord = result.point;
			Vec3f n = result.tri->normal;

			// Fetch bayocentric coords
			float alpha = result.u;
			float beta = result.v;

			// Fetch color from PathTraceRenderer::getTextureParameters()
			Vec3f color;
			getTextureParameters(result, color, h_n, light);

			// To get smooth normals, interpolate the vertex normals to the hit position and normalize it.
			// The normal is interpolated according to the hit barycentrics similarly to texture coordinates.
			Vec2f uv;
			uv[0] = alpha;
			uv[1] = beta;
			Vec3f h_n = getTexelCoords(uv, ???);

			// If this was the first ray from the camera and the hit point is the actual light source, add the emission to the radiance returned by the path
			/*
			if (bounce == 0) {
				// But this doesn't happen in our code so no need to implement it...?
			}
			*/

			// Draw a point from the light source surface, trace a shadow ray, and add the appropriate contribution to the radiance returned by the path. Be careful with the probabilities.
			// Like this??? 
			// From the shader last time...

			// 1/PI, from the diffuse BRDF
			float brdf = (1.0f / m_pi); // *diffuseColor.rgb;

			// Inverse square of the distance from the surface point to the light
			// It might be also -Rd? idk.
			Vec3f l_dir = Rd;
			float i_dist = clamp(1.0f / (l_dir.x*l_dir.x + l_dir.y*l_dir.y + l_dir.z*l_dir.z), 0.0f, 10.0f);

			// Cosine of the angle between the surface normal and the incident lighting direction.
			float cos_s = clamp(dot(-n, l_dir), 0.0f, 1.0f);

			// Cosine of the angle between the light normal and the incident lighting direction.
			float cos_l = clamp(dot(h_n, l_dir), 0.0f, 1.0f);

			// Spotlight emission distribution, i.e. the circular cone.
			// Where do I get lightFOVRad??
			float cone = max(0, min(1, 4 * (dot(normalize(h_n), normalize(l_dir)) - cos(lightFOVRad / 2.0f)) / (1 - cos(lightFOVRad / 2.0f))));
			//float cone = 1.0f;

			// Multiply them all...
			light = light * brdf * i_dist * cos_s * cos_l;

			// Terminate or not?
			// Fixed number of bounces
			if (ctx.m_bounces >= 0 && ctx.m_bounces <= bounce) {
				break;
			}

			// Russian roulette
			if (ctx.m_bounces < 0) {
				// Probability for this round... 20%
				float f = 0.2f;
				float r = R.getF32();

				if (f < r) {
					break;
				}
			}

			//Indirect ray casting
			Vec3f P2;
			float x, y, z;
			while (true) {
				x = R.getF32(-1, 1);
				y = R.getF32(-1, 1);

				if (x*x + y*y <= 1) { break; }
			}

			z = sqrt(1 - x*x - y*y);

			P2 = Vec3f(x, y, z);

			// Conversion to vertex hemisphere
			P2 = B * P2;

			// Make the direction long but not too long to avoid numerical instability in the ray tracer.
			// For our scenes, 100 is a good length. (I know, this special casing sucks.)

			// Get direction, set len to 100
			Vec3f n_dir = P2.normalized() * 100;

			// Shoot ray, see where we hit
			result = ctx.m_rt->raycast(Ro, n_dir);

			bounce++;
		}

		Ei = light;
		//Ei = result.tri->m_material->diffuse.getXYZ(); // placeholder


		if (debugVis)
		{
			// Example code for using the visualization system. You can expand this to include further bounces, 
			// shadow rays, and whatever other useful information you can think of.
			PathVisualizationNode node;
			node.lines.push_back(PathVisualizationLine(result.orig, result.point)); // Draws a line between two points
			node.lines.push_back(PathVisualizationLine(result.point, result.point + result.tri->normal() * .1f, Vec3f(1,0,0))); // You can give lines a color as optional parameter.
			node.labels.push_back(PathVisualizationLabel("diffuse: " + std::to_string(Ei.x) + ", " + std::to_string(Ei.y) + ", " + std::to_string(Ei.z), result.point)); // You can also render text labels with world-space locations.
			
			visualization.push_back(node);
		}
	}
	else {
		Ei = 0;
	}

	return Ei;
}

// This function is responsible for asynchronously generating paths for a given block.
void PathTraceRenderer::pathTraceBlock( MulticoreLauncher::Task& t )
{
    PathTracerContext& ctx = *(PathTracerContext*)t.data;

    const MeshWithColors* scene			= ctx.m_scene;
    RayTracer* rt						= ctx.m_rt;
    Image* image						= ctx.m_image.get();
    const CameraControls& cameraCtrl	= *ctx.m_camera;
    AreaLight* light					= ctx.m_light;

    // make sure we're on CPU
    image->getMutablePtr();

    // get camera orientation and projection
    Mat4f worldToCamera = cameraCtrl.getWorldToCamera();
    Mat4f projection = Mat4f::fitToView(Vec2f(-1,-1), Vec2f(2,2), image->getSize())*cameraCtrl.getCameraToClip();

    // inverse projection from clip space to world space
    Mat4f invP = (projection * worldToCamera).inverted();

    // get the block which we are rendering
    PathTracerBlock& block = ctx.m_blocks[t.idx];

	// Not used but must be passed to tracePath
	std::vector<PathVisualizationNode> dummyVisualization; 

	static std::atomic<uint32_t> seed = 0;
	uint32_t current_seed = seed.fetch_add(1);
	Random R(t.idx + current_seed);	// this is bogus, just to make the random numbers change each iteration

    for ( int i = 0; i < block.m_width * block.m_height; ++i )
    {
        if( ctx.m_bForceExit ) {
            return;
        }

        // Use if you want.
        int pixel_x = block.m_x + (i % block.m_width);
        int pixel_y = block.m_y + (i / block.m_width);

		Vec3f Ei = tracePath(pixel_x, pixel_y, ctx, 0, R, dummyVisualization);

        // Put pixel.
        Vec4f prev = image->getVec4f( Vec2i(pixel_x, pixel_y) );
        prev += Vec4f( Ei, 1.0f );
        image->setVec4f( Vec2i(pixel_x, pixel_y), prev );
    }
}

void PathTraceRenderer::startPathTracingProcess( const MeshWithColors* scene, AreaLight* light, RayTracer* rt, Image* dest, int bounces, const CameraControls& camera )
{
    FW_ASSERT( !m_context.m_bForceExit );

    m_context.m_bForceExit = false;
    m_context.m_bResidual = false;
    m_context.m_camera = &camera;
    m_context.m_rt = rt;
    m_context.m_scene = scene;
    m_context.m_light = light;
    m_context.m_pass = 0;
    m_context.m_bounces = bounces;
    m_context.m_image.reset(new Image( dest->getSize(), ImageFormat::RGBA_Vec4f));

    m_context.m_destImage = dest;
    m_context.m_image->clear();

    // Add rendering blocks.
    m_context.m_blocks.clear();
    {
        int block_size = 32;
        int image_width = dest->getSize().x;
        int image_height = dest->getSize().y;
        int block_count_x = (image_width + block_size - 1) / block_size;
        int block_count_y = (image_height + block_size - 1) / block_size;

        for(int y = 0; y < block_count_y; ++y) {
            int block_start_y = y * block_size;
            int block_end_y = FW::min(block_start_y + block_size, image_height);
            int block_height = block_end_y - block_start_y;

            for(int x = 0; x < block_count_x; ++x) {
                int block_start_x = x * block_size;
                int block_end_x = FW::min(block_start_x + block_size, image_width);
                int block_width = block_end_x - block_start_x;

                PathTracerBlock block;
                block.m_x = block_size * x;
                block.m_y = block_size * y;
                block.m_width = block_width;
                block.m_height = block_height;

                m_context.m_blocks.push_back(block);
            }
        }
    }

    dest->clear();

    // Fire away!

    // If you change this, change the one in checkFinish too.
    m_launcher.setNumThreads(m_launcher.getNumCores());
    //m_launcher.setNumThreads(1);

    m_launcher.popAll();
    m_launcher.push( pathTraceBlock, &m_context, 0, (int)m_context.m_blocks.size() );
}

void PathTraceRenderer::updatePicture( Image* dest )
{
    FW_ASSERT( m_context.m_image != 0 );
    FW_ASSERT( m_context.m_image->getSize() == dest->getSize() );

    for ( int i = 0; i < dest->getSize().y; ++i )
    {
        for ( int j = 0; j < dest->getSize().x; ++j )
        {
            Vec4f D = m_context.m_image->getVec4f(Vec2i(j,i));
            if ( D.w != 0.0f )
                D = D*(1.0f/D.w);

            // Gamma correction.
            Vec4f color = Vec4f(
                FW::pow(D.x, 1.0f / 2.2f),
                FW::pow(D.y, 1.0f / 2.2f),
                FW::pow(D.z, 1.0f / 2.2f),
                D.w
            );

            dest->setVec4f( Vec2i(j,i), color );
        }
    }
}

void PathTraceRenderer::checkFinish()
{
    // have all the vertices from current bounce finished computing?
    if ( m_launcher.getNumTasks() == m_launcher.getNumFinished() )
    {
        // yes, remove from task list
        m_launcher.popAll();

        ++m_context.m_pass;

        // you may want to uncomment this to write out a sequence of PNG images
        // after the completion of each full round through the image.
        //String fn = sprintf( "pt-%03dppp.png", m_context.m_pass );
        //File outfile( fn, File::Create );
        //exportLodePngImage( outfile, m_context.m_destImage );

        if ( !m_context.m_bForceExit )
        {
            // keep going

            // If you change this, change the one in startPathTracingProcess too.
            m_launcher.setNumThreads(m_launcher.getNumCores());
            //m_launcher.setNumThreads(1);

            m_launcher.popAll();
            m_launcher.push( pathTraceBlock, &m_context, 0, (int)m_context.m_blocks.size() );
            //::printf( "Next pass!" );
        }
        else ::printf( "Stopped." );
    }
}

void PathTraceRenderer::stop() {
    m_context.m_bForceExit = true;
    
    if ( isRunning() )
    {
        m_context.m_bForceExit = true;
        while( m_launcher.getNumTasks() > m_launcher.getNumFinished() )
        {
            Sleep( 1 );
        }
        m_launcher.popAll();
    }

    m_context.m_bForceExit = false;
}



} // namespace FW
