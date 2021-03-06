#include "OpenGLParticle.h"
#include <glad/glad.h>
#include <PerlinNoise.hpp>
#include "particle_vert.h"
#include "particle_geom.h"
#include "particle_frag.h"

#include<cstdlib>
#include "Core/Base/macro.h"
namespace Stone
{
	OpenGLParticle::OpenGLParticle()
	{
		initRandomTexture(1000);
		m_UniformBuffer = UniformBuffer::create();
		m_UniformBuffer->setData(&m_ParticleGlobalData, sizeof(m_ParticleGlobalData));
		m_VBO1 = VertexBuffer::create(MAX_PARTICLE_NUM * sizeof(Particle));
		m_VBO1->setLayout({
			{ ShaderDataType::Float,	"in_Type" },
			{ ShaderDataType::Float3,	"in_Position" },
			{ ShaderDataType::Float3,	"in_Velocity" },
			{ ShaderDataType::Float,	"in_Age" }
			});
		m_VBO2 = VertexBuffer::create(MAX_PARTICLE_NUM * sizeof(Particle));
		m_VBO2->setLayout({
			{ ShaderDataType::Float,	"in_Type" },
			{ ShaderDataType::Float3,	"in_Position" },
			{ ShaderDataType::Float3,	"in_Velocity" },
			{ ShaderDataType::Float,	"in_Age" }
			});

		m_VAO1 = VertexArray::create();
		m_VAO2 = VertexArray::create();

		m_VAO1->addVertexBuffer(m_VBO1);
		m_VAO2->addVertexBuffer(m_VBO2);
		glGenQueries(1, &m_Query);

		m_ParticleShader = Shader::create("m_ParticleShader");
		auto vershader = m_ParticleShader->create(particle_vert, sizeof(particle_vert), Shader::ShaderType::Vertex_Shader);
		m_ParticleShader->attach(vershader);
		auto geoshader = m_ParticleShader->create(particle_geom, sizeof(particle_geom), Shader::ShaderType::Geometry_Shader);
		m_ParticleShader->attach(geoshader);
		const GLchar* feedbackVaryings[] = { "out_Type", "out_Position", "out_Velocity", "out_Age"};
		glTransformFeedbackVaryings(m_ParticleShader->getRenderID(), 4, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
		auto fragshader = m_ParticleShader->create(particle_frag, sizeof(particle_frag), Shader::ShaderType::Fragment_Shader);
		m_ParticleShader->attach(fragshader);
		m_ParticleShader->link();
	}
	void OpenGLParticle::logictick()
	{
		if (m_Particles.size()==0)
		{
			return;
		}
		m_ParticleShader->bind();
		m_Primitives = m_IsFirst ? m_Particles.size() : m_Primitives;
		glEnable(GL_RASTERIZER_DISCARD);
		m_ParticleShader->setInt("randomTexture", 0);
		bindRandomTexture(0);
		m_UniformBuffer->bind(5);
	 	m_SwapFlag ? m_VAO1->bind() : m_VAO2->bind();
		m_SwapFlag ? m_VBO2->bindTransformFeedback(0) : m_VBO1->bindTransformFeedback(0);
		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_Query);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, m_Primitives);
		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

		glFlush();

		glGetQueryObjectuiv(m_Query, GL_QUERY_RESULT, &m_Primitives);
#if 0
		GLfloat feedback[5];
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
		for (size_t i = 0; i < 5; i++)
		{
		    LOG_DEBUG("feedback: {0}", feedback[i]);
		}
		LOG_DEBUG("{0} primitives written!", m_Primitives);
#endif
		glDisable(GL_RASTERIZER_DISCARD);

		m_SwapFlag = ! m_SwapFlag;
		m_IsFirst = false;
	}
	void OpenGLParticle::rendertick()
	{
		if (m_Particles.size() == 0)
		{
			return;
		}
		m_ParticleShader->bind();
		m_SwapFlag ? m_VAO1->bind() : m_VAO2->bind();
		glDrawArrays(GL_POINTS, 0, m_Primitives);
		m_SwapFlag != m_SwapFlag;
	}
	void OpenGLParticle::add(const std::vector<Particle>& particles)
	{
		m_VBO1->setData(particles.data(), MAX_PARTICLE_NUM * sizeof(Particle));
	}
	void OpenGLParticle::add(const Particle& particle)
	{
		m_Particles.push_back(particle);
		m_VBO1->setData((void*)m_Particles.data(), MAX_PARTICLE_NUM * sizeof(Particle));
	}
	void OpenGLParticle::initRandomTexture(uint32_t size)
	{
		glm::vec3* pRandomData = new glm::vec3[size];
		for (unsigned int i = 0; i < size; i++) {
			pRandomData[i].x = rand() / double(RAND_MAX);
			pRandomData[i].y = rand() / double(RAND_MAX);
			pRandomData[i].z = rand() / double(RAND_MAX);
		}

		glGenTextures(1, &m_RandomTextureId);
		glBindTexture(GL_TEXTURE_1D, m_RandomTextureId);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, pRandomData);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		delete[] pRandomData;
	}
	void OpenGLParticle::bindRandomTexture(uint32_t index)
	{
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_1D, m_RandomTextureId);
	}
}