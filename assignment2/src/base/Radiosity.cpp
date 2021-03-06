#include "Radiosity.hpp"
#include "AreaLight.hpp"
#include "RayTracer.hpp"



namespace FW {


// --------------------------------------------------------------------------

Radiosity::~Radiosity()
{
    if ( isRunning() )
    {
        m_context.m_bForceExit = true;
        while( m_launcher.getNumTasks() > m_launcher.getNumFinished() )
            Sleep( 1 );
        m_launcher.popAll();
    }
}


// --------------------------------------------------------------------------
void Radiosity::vertexTaskFunc( MulticoreLauncher::Task& task )
{
    RadiosityContext& ctx = *(RadiosityContext*)task.data;

    if( ctx.m_bForceExit )
        return;

    // which vertex are we to compute?
    int v = task.idx;

    // fetch vertex and its normal
    Vec3f n = ctx.m_scene->vertex(v).n.normalized();
    Vec3f o = ctx.m_scene->vertex(v).p + 0.01f*n;

    // YOUR CODE HERE (R3):
    // This starter code merely puts the color-coded normal into the result.
    //
    // In the first bounce, your task is to compute the direct irradiance
    // falling on this vertex from the area light source.
    // In the subsequent passes, you should compute the irradiance by a
    // hemispherical gathering integral. The commented code below gives you
    // an idea of the loop structure. Note that you also have to account
    // for how diffuse textures modulate the irradiance.
	/*
    ctx.m_vecResult[ v ] = n*0.5+0.5;
    Sleep(1);
    return;
	*/

	Random rnd(v);
    
    // direct lighting pass? => integrate direct illumination by shooting shadow rays to light source
    if ( ctx.m_currentBounce == 0 )
    {
        Vec3f E(0);
        for ( int r = 0; r < ctx.m_numDirectRays; ++r )
        {
            // draw sample on light source
            float pdf;
            Vec3f Pl;

            // construct vector from current vertex (o) to light sample
			ctx.m_light->sample(pdf, Pl, 1, rnd);
			Vec3f n_dir = Pl - o;
			auto cast_res = ctx.m_rt->raycast(o, n_dir);

            // trace shadow ray to see if it's blocked
            if (cast_res.tri == nullptr){
                // if not, add the appropriate emission, 1/r^2 and clamped cosine terms, accounting for the PDF as well.
                // accumulate into E

				// Where to get T??
				// E += E*T;
				float a_cos = clamp(dot(-ctx.m_light->getNormal(), n_dir.normalized()), 0.0f, 1.0f);
				float t_cos = clamp(dot(n_dir.normalized(), n), 0.0f, 1.0f);
				E += (ctx.m_light->getEmission() / n_dir.lenSqr() * a_cos * t_cos) / pdf;
            }
        }
        // Note we are NOT multiplying by PI here;
        // it's implicit in the hemisphere-to-light source area change of variables.
        // The result we are computing is _irradiance_, not radiosity.
        ctx.m_vecCurr[ v ] = E * (1.0f/ctx.m_numDirectRays);
        ctx.m_vecResult[ v ] = ctx.m_vecCurr[ v ];
    } 
    else
    {
        // OK, time for indirect!
        // Implement hemispherical gathering integral for bounces > 1.

        // Get local coordinate system the rays are shot from.
        Mat3f B = formBasis( n );

        Vec3f E(0.0f);
        for ( int r = 0; r < ctx.m_numHemisphereRays; ++r )
        {
            // Draw a cosine weighted direction and find out where it hits (if anywhere)
            // You need to transform it from the local frame to the vertex' hemisphere using B.

			/*
			float pdf = cos(n, dir) / pi;
			prdf = cos() * diffusion / pi;
			E = prdf/pdf
				-> E = diffusion * l_in*/

			Vec3f P2;

			// This... probably isn't right? Since this is (cosl(t)*cos(t)/r^2)*V
			// We need just one cos... so we get cos(t)/FW_PI
			float x, y, z;
			while (true) {
				x = rnd.getF32(-1, 1);
				y = rnd.getF32(-1, 1);

				if (x*x + y*y <= 1) { break; }
			}

			z = sqrt(1 - x*x - y*y);

			P2 = Vec3f(x, y, z);

			// Conversion to vertex hemisphere
			P2 = B * P2;

            // Make the direction long but not too long to avoid numerical instability in the ray tracer.
            // For our scenes, 100 is a good length. (I know, this special casing sucks.)

			// Get direction, set len to 100
			Vec3f n_dir = P2.normalized()*100;

            // Shoot ray, see where we hit
            const RaycastResult result = ctx.m_rt->raycast( o, n_dir );
            if ( result.tri != nullptr )
            {
                // interpolate lighting from previous pass
				const Vec3i& indices = result.tri->m_data.vertex_indices;

                // check for backfaces => don't accumulate if we hit a surface from below!
				if (dot(result.tri->normal(), result.dir) > 0) { continue; }

                // fetch barycentric coordinates
				float alpha = result.u;
				float beta = result.v;

				// Vec2f bayo_y = alpha * result.tri->m_vertices[0].t + beta * result.tri->m_vertices[1].t + (1 - alpha - beta) * result.tri->m_vertices[2].t;
				// what was I supposed to use this for again?

                // Ei = interpolated irradiance determined by ctx.m_vecPrevBounce from vertices using the barycentric coordinates
				// Is it really this simple? 
				Vec3f Ei = alpha * ctx.m_vecPrevBounce[indices[1]] + beta * ctx.m_vecPrevBounce[indices[2]] + (1 - alpha - beta) * ctx.m_vecPrevBounce[indices[0]];

                // Divide incident irradiance by PI so that we can turn it into outgoing
                // radiosity by multiplying by the reflectance factor below.
                Ei *= (1.0f / FW_PI);

                // check for texture
                const auto mat = result.tri->m_material;
                if ( mat->textures[MeshBase::TextureType_Diffuse].exists() )
                {
					
					// read diffuse texture like in assignment1

                    const Texture& tex = mat->textures[MeshBase::TextureType_Diffuse];
                    const Image& teximg = *tex.getImage();

					Vec2f uv;
					uv[0] = alpha;
					uv[1] = beta;
					Vec2i texelCoords = getTexelCoords(uv, teximg.getSize());

					// Which bounce vec is this?
					// Do I need to do this to them all?
					auto diffuse = teximg.getVec4f(texelCoords).getXYZ();
					Ei *= diffuse;
                }
                else
                {
                    // no texture, use constant albedo from material structure.

                    // (this is just one line)
					Ei *= mat->diffuse.getXYZ();
                }

                E += Ei;	// accumulate
            }
        }
        // Store result for this bounce
        // Note that since we are storing irradiance, we multiply by PI(
        // (Remember the slides about cosine weighted importance sampling!)
        ctx.m_vecCurr[ v ] = E * (FW_PI / ctx.m_numHemisphereRays);
        // Also add to the global accumulator.
        ctx.m_vecResult[ v ] = ctx.m_vecResult[ v ] + ctx.m_vecCurr[ v ];

        // uncomment this to visualize only the current bounce
        //ctx.m_vecResult[ v ] = ctx.m_vecCurr[ v ];	
    }
}
// --------------------------------------------------------------------------

void Radiosity::startRadiosityProcess( MeshWithColors* scene, AreaLight* light, RayTracer* rt, int numBounces, int numDirectRays, int numHemisphereRays )
{
    // put stuff the asyncronous processor needs 
    m_context.m_scene				= scene;
    m_context.m_rt					= rt;
    m_context.m_light				= light;
    m_context.m_currentBounce		= 0;
    m_context.m_numBounces			= numBounces;
    m_context.m_numDirectRays		= numDirectRays;
    m_context.m_numHemisphereRays	= numHemisphereRays;

    // resize all the buffers according to how many vertices we have in the scene
	m_context.m_vecResult.resize(scene->numVertices());
    m_context.m_vecCurr.resize( scene->numVertices() );
    m_context.m_vecPrevBounce.resize( scene->numVertices() );
    m_context.m_vecResult.assign( scene->numVertices(), Vec3f(0,0,0) );

	m_context.m_vecSphericalC.resize(scene->numVertices());
	m_context.m_vecSphericalX.resize(scene->numVertices());
	m_context.m_vecSphericalY.resize(scene->numVertices());
	m_context.m_vecSphericalZ.resize(scene->numVertices());

	m_context.m_vecSphericalC.assign(scene->numVertices(), Vec3f(0, 0, 0));
	m_context.m_vecSphericalX.assign(scene->numVertices(), Vec3f(0, 0, 0));
	m_context.m_vecSphericalY.assign(scene->numVertices(), Vec3f(0, 0, 0));
	m_context.m_vecSphericalZ.assign(scene->numVertices(), Vec3f(0, 0, 0));

    // fire away!
    m_launcher.setNumThreads(m_launcher.getNumCores());	// the solution exe is multithreaded
    //m_launcher.setNumThreads(1);							// but you have to make sure your code is thread safe before enabling this!
    m_launcher.popAll();
    m_launcher.push( vertexTaskFunc, &m_context, 0, scene->numVertices() );
}
// --------------------------------------------------------------------------

void Radiosity::updateMeshColors(std::vector<Vec4f>& spherical1, std::vector<Vec4f>& spherical2, std::vector<float>& spherical3, bool spherical)
{
    // Print progress.
    printf( "%.2f%% done     \r", 100.0f*m_launcher.getNumFinished()/m_context.m_scene->numVertices() );

    // Copy irradiance over to the display mesh.
    // Because we want outgoing radiosity in the end, we divide by PI here
    // and let the shader multiply the final diffuse reflectance in. See App::setupShaders() for details.
	for (int i = 0; i < m_context.m_scene->numVertices(); ++i) {

		// Packing data for the spherical harmonic extra.
		// In order to manage with fewer vertex attributes in the shader, the third component is stored as the w components of other actually three-dimensional vectors.
		if (spherical) {
			auto mult = (2.0f / FW_PI);
			m_context.m_scene->mutableVertex(i).c = m_context.m_vecSphericalC[i] *mult;
			spherical3[i] = m_context.m_vecSphericalZ[i].x *mult;
			spherical1[i] = Vec4f(m_context.m_vecSphericalX[i], m_context.m_vecSphericalZ[i].y) *mult;
			spherical2[i] = Vec4f(m_context.m_vecSphericalY[i], m_context.m_vecSphericalZ[i].z) *mult;
		}
		else {
			m_context.m_scene->mutableVertex(i).c = m_context.m_vecResult[i] * (1.0f / FW_PI);
		}
	}
}
// --------------------------------------------------------------------------

void Radiosity::checkFinish()
{
    // have all the vertices from current bounce finished computing?
    if ( m_launcher.getNumTasks() == m_launcher.getNumFinished() )
    {
        // yes, remove from task list
        m_launcher.popAll();

        // more bounces desired?
        if ( m_context.m_currentBounce < m_context.m_numBounces )
        {
            // move current bounce to prev
            m_context.m_vecPrevBounce = m_context.m_vecCurr;
            ++m_context.m_currentBounce;
            // start new tasks for all vertices
            m_launcher.push( vertexTaskFunc, &m_context, 0, m_context.m_scene->numVertices() );
            printf( "\nStarting bounce %d\n", m_context.m_currentBounce );
        }
        else printf( "\n DONE!\n" );
    }
}
// --------------------------------------------------------------------------

} // namespace FW
