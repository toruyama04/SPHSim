#include "Firework.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <vector>



float vertices[] = {
    // positions
     0.01f,  0.01f, 0.0f,
     0.01f, -0.01f, 0.0f,
    -0.01f, -0.01f, 0.0f,
    -0.01f,  0.01f, 0.0f
};

unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

Firework::Firework()
{
    firework_lifetime = 3.0f;
    _particle_num = 10240;
    _max_particles = 102400;

    // set grid size (parameters)

	initBuffers();
	initShaders();
    // initSolvers();
}

Firework::~Firework()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &positionsSSBO);
    glDeleteBuffers(1, &velocitiesSSBO);
    glDeleteBuffers(1, &aliveFlagSSBO);
    glDeleteBuffers(1, &atomicCounterBuffer);
    glDeleteBuffers(1, &drawIndirectBuffer);
}

void Firework::initSolvers()
{
    // GridFluidSolver initialise
    _gridSpacing = { 1, 1, 1 };
    _gridOrigin = { 0, 0, 0 };
    _velocity = std::make_shared<FaceCenteredGrid3>();
    _advectableVectorDataList.push_back(_velocity);
    _velocityIdx = 0;
    // for each _scalarDataList, _vectorDataList, _advectableScalarDatalist, _advectableVectorDataList
    // resize to 'resolution, gridSpacing, origin'

    // set advectionSolver
    //! _advectionSolver = std::make_unique<SemiLangrangian/CubicSemiLagrangian3>();
    // set diffusionSolver
    //! _diffusionSolver = std::make_unqiue<GridBackwardEulerDiffusion/GridForwardEulerDiffusionSolver3>();
    // set pressureSolver
    //! _pressureSolver = std::make_unique<GridFractionalSinglePhasePressure/GridSinglePhasePressureSolver3>();
    // add scalarGrids for smoke, maybe temp

    // _smokeDensityId = addAdvectableScaleData() initial = 0.0;
    // temperature ?

}

void Firework::initShaders()
{
    shaders["particleShader"] = std::make_unique<Shader>("shaders/particle.vert", "shaders/particle.frag");
    shaders["computeShaderUpdate"] = std::make_unique<Shader>("shaders/particleUpdate.comp");
    // shaders["computeShaderPrefix"] = std::make_unique<Shader>("shaders/particlePrefix.comp");
    // shaders["computeShaderTrail"] = std::make_unique<Shader>("shaders/particleTrail.comp");
    // shaders["computeShaderReorder"] = std::make_unique<Shader>("shaders/particleReorder.comp");
    //shaders["computeShaderDenPre"] = std::make_unique<Shader>("shaders/particleDenPre.comp");
}

void Firework::initBuffers()
{
    // initialising firework VAO
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &atomicCounterBuffer);
    glGenBuffers(1, &positionsSSBO);
    glGenBuffers(1, &velocitiesSSBO);
    glGenBuffers(1, &aliveFlagSSBO);
	glGenBuffers(1, &drawIndirectBuffer);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &forcesSSBO);
    glGenBuffers(1, &ScalarData);
    glGenBuffers(1, &VectorData);
    glGenBuffers(1, &newPositionSSBO);
    glGenBuffers(1, &newVelocitySSBO);
	glBindVertexArray(VAO);

    // initialising particle vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    /*glGenTextures(1, &particleTexture);
    glBindTexture(GL_TEXTURE_2D, particleTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("smoke.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        std::cout << "Failed to load texture\n";
    stbi_image_free(data);*/

    glBindVertexArray(0);

    // initialising alive count to 0 for atomic counter buffer
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint initial = 0;
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // initialising positions SSBO with default values
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    std::vector default_values(_max_particles, glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialising velocity SSBO with default values
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newPositionSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, newPositionSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newVelocitySSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, newVelocitySSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialising alive flag SSBO with default values
    std::vector<int> flag_default(_max_particles, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, aliveFlagSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * _max_particles, flag_default.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, aliveFlagSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // create forces SSBO for density, pressure, viscosity, mass
    /*
     * x = density
     * y = pressure
     * z = viscosity
     * w = mass
     * */
    std::vector<glm::vec4> forceDefaults(_max_particles, glm::vec4(-1.0, -1.0, -1.0, 1e-3));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forcesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, forceDefaults.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, forcesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // scalar data buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ScalarData);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(double) * _max_particles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ScalarData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // vector data buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VectorData);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * _max_particles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, VectorData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialising indirect buffer
    /*glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    cmd = { 6, 1, 0, 0, 0};
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);*/

	/*if (add_floor)
    {
        glGenVertexArrays(1, &floorVAO);
        glGenBuffers(1, &floorVBO);

        glBindVertexArray(floorVAO);

        glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, floorVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }*/
}

void Firework::render(const glm::mat4& view, const glm::mat4& projection)
{
    GLuint count = getAliveCount();
    cmd = { 6, count, 0, 0, 0 };
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(cmd), &cmd);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    shaders["particleShader"]->use();
	glm::mat4 model = glm::mat4(1.0f);
    shaders["particleShader"]->setMat4("model", model);
    shaders["particleShader"]->setMat4("view", view);
    shaders["particleShader"]->setMat4("projection", projection);

    if (count > 0)
    {
		glBindVertexArray(VAO);
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
	glBindVertexArray(0);

    /*
     * to render 3D fluids, the surface was extracted by taking the iso-surface from the SPH density field.
     * If fluid density is p, p/2 is taken for the isoSurface.
     * then converted to a triangle mesh using marching cubes algorithm
     * rendered using a path-tracing renderer
     */


    /*shaders["floorShader"]->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.0f));
    shaders["floorShader"]->setMat4("model", model);
    shaders["floorShader"]->setMat4("view", view);
    shaders["floorShader"]->setMat4("projection", projection);*/
}

void Firework::update(float delta_time)
{
    resetAliveCount();
    /*shaders["computeShaderDenPre"]->use();
    shaders["computeShaderDenPre"]->setFloat("smoothing_length", 1.0f);
    shaders["computeShaderDenPre"]->setFloat("rest_density", 1000.0f);
    shaders["computeShaderDenPre"]->setFloat("pressure_stiffness", 2000.0f);
    glDispatchCompute((cmd.instanceCount + 255) / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

    // update particle data (seems to always call applyBoundaryCondition() after each step)
	// beginAdvanceTimeStep();
    // resets force
    // find neighbours, store neighbour list
    // updateDensities

    // accumulateNonPressureForce
    /**
     * for each particle:
     *      reset force 
     *      force = mass * _gravity;
     *      forces[i] += force;
     *      using spiky kernel to calculate the viscosity
     *      compute pressure from eos in physics_helpers.h
     *
     *  uniform: eos_exponent, negative_pressureScale, viscosit coefficient, psuedoViscosity coefficient, speedofsound, + more probs
    */

    // accumulatePressureForce
    /**
     * using spiky kernel, for each particle, for each neighbour, adjust their pressureForces[]
     *
     */

    // timeIntegration
    /**
     * for each particle:
     *      set newVelocity = oldVelocity + dt * forces[i] / mass
     *      set newPosition = oldPosition + dt * newVelocity
     */

    // endAdvanceTimeStep();
    /*
     * set oldPosition = newPosition, oldVelocity = newVelocity
     */

    shaders["computeShaderUpdate"]->use();
    shaders["computeShaderUpdate"]->setFloat("deltaTime", delta_time);
    shaders["computeShaderUpdate"]->setVec3("grav", gravity);
    shaders["computeShaderUpdate"]->setFloat("noiseScale", 0.8f); // Scale for noise field
    shaders["computeShaderUpdate"]->setFloat("noiseSpeed", 0.4f); // Speed for noise field
    shaders["computeShaderUpdate"]->setFloat("dampingFactor", 0.995f); // Damping factor
    glDispatchCompute((_max_particles + 255) / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    
    // prefix sum
    /*shaders["computeShaderPrefix"]->use();
    glDispatchCompute((_max_particles + 1023) / 1024, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

    // reorder
    /*shaders["computeShaderReorder"]->use();
    glDispatchCompute((_max_particles + 1023) / 1024, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

    // add trail particles
    /** what happens when we add more fireworks?
     * may need to add trails before prefix/reorder - make sure to flag trails as alive when adding
     * change dispatch to the number of alive particles (getAliveCount())
     * in computeShader, add another or extend aliveFlag SSBO to indicate whether trail or not
     * change particleTrail so it adds to aliveCount if the aliveparticle is a firework particle, else return early
     */
    /* second issue: since aliveCount contains trail particles, they are being updated as normal
     *      however, they are not differentiated from firework particles. May just include another
     *      SSBO for trail or not and another atomic counter for finding the right index to draw
     *      the trail particle.
     */
    /*shaders["computeShaderTrail"]->use();
    shaders["computeShaderTrail"]->setFloat("trailRate", 0.9f);
    shaders["computeShaderTrail"]->setUInt("maxParticle", _max_particles);
    glDispatchCompute((particle_num + 255) / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

    /*glBindVertexArray(floorVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);	*/
}

void Firework::resetAliveCount()
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint initial = 0;
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &initial);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

// add default forces to firework
void Firework::addFirework(const glm::vec3& origin)
{
    GLuint aliveIndex = getAliveCount();
    if (aliveIndex + _particle_num < _max_particles)
    {
        ++_firework_num;
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocity;
        for (unsigned int j = 0; j < _particle_num; ++j)
        {
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(10.0f, 15.0f);
            velocity.emplace_back(speed * sin(phi) * cos(theta), speed * sin(phi) * sin(theta), speed * cos(phi), firework_lifetime);
            positions.emplace_back(origin, firework_lifetime);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * _particle_num, positions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * _particle_num, velocity.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        // change binding point number for each firework
        _neighbour_grids.push_back(std::make_unique<PointHashGridSearcher3>(50, 100, 50, 2.0, _particle_num, 8));
    }
    else
        std::cout << "exceeded max particle num\n";
    
}

GLuint Firework::getAliveCount()
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint* num = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    GLuint count = num[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    return count;
}

