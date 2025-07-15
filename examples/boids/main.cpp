#include <OpenGLLib/core.h>
#include <random>
#include <memory>
#include <iostream>

#include "new_buffer.h"
#include "settings.h"

using namespace glm;
using std::make_unique;
using std::unique_ptr;
using std::vector;
using distribution = std::uniform_real_distribution<float>;
using random = std::default_random_engine;

vec2 clampLength(vec2 v, float maxLength)
{
    float length = glm::length(v);
    if (length > maxLength)
    {
        return v * (maxLength / length);
    }
    return v;
}

vec2 steerTorwards(vec2 current, vec2 target, float maxSpeed, float maxForce)
{
    vec2 desired = normalize(target) * maxSpeed;
    vec2 steer = desired - current;
    return clampLength(steer, maxForce);
}

float angleBetween(vec2 a, vec2 b)
{
    return degrees(acos(dot(normalize(a), normalize(b))));
}

struct Boid
{
    vec2 position;
    vec2 velocity;
};

class Simulation
{
public:
    Simulation(Settings *settings) : settings(settings) {}
    virtual void update(float deltaTime, vector<Boid> *boids, GLBuffer *writer, GLBuffer *reader) = 0;
    virtual void sync(vector<Boid> *boids, GLBuffer *writer, GLBuffer *reader) = 0;

protected:
    Settings *settings;
};

class CPUSimulation : public Simulation
{
public:
    CPUSimulation(Settings *settings) : Simulation(settings) {}
    void update(float deltaTime, vector<Boid> *boids, GLBuffer *writer, GLBuffer *reader) override
    {
        vector<Boid> newBoids = *boids;
        for (int i = 0; i < newBoids.size(); ++i)
        {
            Boid &boid = newBoids[i];
            vec2 alignment(0.0f);
            vec2 cohesion(0.0f);
            vec2 separation(0.0f);
            int flockSize = 0;
            int closeSize = 0;

            for (int j = 0; j < newBoids.size(); ++j)
            {
                if (i == j)
                    continue;

                Boid &other = (*boids)[j];
                vec2 toOther = other.position - boid.position;
                float angle = angleBetween(boid.velocity, toOther);
                float distance = length(toOther);

                if (angle > settings->angleView / 2.0f)
                    continue;

                if (distance < settings->perceptionRadius)
                {
                    alignment += other.velocity;
                    cohesion += other.position;
                    flockSize++;
                    if (distance < settings->avoidanceRadius)
                    {
                        separation -= toOther / (distance * distance); // Avoidance
                        closeSize++;
                    }
                }
            }

            vec2 acceleration(0.0f);
            if (flockSize > 0)
            {
                alignment /= flockSize;
                cohesion /= flockSize;
                cohesion -= boid.position;

                acceleration += steerTorwards(boid.velocity, alignment, settings->maxSpeed, settings->maxForce) * settings->alignmentWeight;
                acceleration += steerTorwards(boid.velocity, cohesion, settings->maxSpeed, settings->maxForce) * settings->cohesionWeight;
                if (closeSize > 0)
                {
                    acceleration += steerTorwards(boid.velocity, separation, settings->maxSpeed, settings->maxForce) * settings->separationWeight;
                }
            }

            // Update velocity
            boid.velocity += acceleration * deltaTime;
            boid.velocity = clampLength(boid.velocity, settings->maxSpeed);
            boid.position += boid.velocity * deltaTime;

            // Wrap around screen edges
            if (boid.position.x < 0)
                boid.position.x += settings->screenSize.x;
            else if (boid.position.x > settings->screenSize.x)
                boid.position.x -= settings->screenSize.x;

            if (boid.position.y < 0)
                boid.position.y += settings->screenSize.y;
            else if (boid.position.y > settings->screenSize.y)
                boid.position.y -= settings->screenSize.y;
        }

        *boids = std::move(newBoids);
    };

    void sync(vector<Boid> *boids, GLBuffer *writer, GLBuffer *reader) override
    {
        // Write updated boids to the writer buffer
        writer->bind();
        writer->setData(sizeof(Boid) * boids->size(), boids->data(), GL_DYNAMIC_DRAW);
        writer->unbind();
    }
};

class GPUSimulation : public Simulation
{
public:
    GPUSimulation(Settings *settings) : Simulation(settings), kernel(SHADER_PATH "compute.glsl")
    {
    }

    void update(float deltaTime, vector<Boid> *boids, GLBuffer *writer, GLBuffer *reader) override
    {
        reader->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, reader->id());
        writer->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, writer->id());

        kernel.use();
        kernel.global[0] = (settings->amount + 255) / 256; // Ensure enough work groups
        kernel.set("deltaTime", deltaTime);
        kernel.set("screenSize", settings->screenSize);
        kernel.set("amount", settings->amount);
        kernel.set("maxSpeed", settings->maxSpeed);
        kernel.set("maxForce", settings->maxForce);
        kernel.set("alignmentWeight", settings->alignmentWeight);
        kernel.set("cohesionWeight", settings->cohesionWeight);
        kernel.set("separationWeight", settings->separationWeight);
        kernel.set("angleView", settings->angleView);
        kernel.set("perceptionRadius", settings->perceptionRadius);
        kernel.set("avoidanceRadius", settings->avoidanceRadius);

        kernel.execute();

        // check errors
        // GLenum err;
        // while ((err = glGetError()) != GL_NO_ERROR)
        // {
        //     std::cerr << "OpenGL error: " << err << std::endl;
        // }
    }

    void sync(vector<Boid> *boids, GLBuffer *writer, GLBuffer *reader) override
    {
        // Read updated boids from the writer buffer
        kernel.release();
        writer->bind();
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Boid) * boids->size(), boids->data());
        writer->unbind();
    }

private:
    ComputeShader kernel;
};

class MyApp : public App
{

public:
    MyApp() : App(4, 3, 800, 800, "Boids Simulation"), model(BufferType::Vertex), shader(SHADER_PATH "vert.glsl", SHADER_PATH "frag.glsl")
    {
        glfwSwapInterval(0); // Disable VSync
        settings.screenSize = vec2(width_, height_);
        boids.resize(settings.amount);
        random rng;
        distribution distX(0, settings.screenSize.x);
        distribution distY(0, settings.screenSize.y);
        distribution distVel(-1, 1);
        for (auto &boid : boids)
        {
            boid.position = vec2(distX(rng), distY(rng));
            boid.velocity = normalize(vec2(distVel(rng), distVel(rng))) * settings.maxSpeed;
        }

        writer = make_unique<GLBuffer>(BufferType::Storage);
        writer->setData(sizeof(Boid) * boids.size(), boids.data(), GL_DYNAMIC_DRAW);
        reader = make_unique<GLBuffer>(BufferType::Storage);
        reader->setData(sizeof(Boid) * boids.size(), boids.data(), GL_DYNAMIC_DRAW);

        // triangle model
        float vertices[] = {
            -0.5f,
            -0.5f,
            0.5f,
            -0.5f,
            0.0f,
            0.5f,
        };

        model.setData(sizeof(vertices), vertices, GL_STATIC_DRAW);

        va.bindBuffer(model)
            .push<float>(2, false) // position
            .apply();

        simulations.push_back(make_unique<CPUSimulation>(&settings));
        simulations.push_back(make_unique<GPUSimulation>(&settings));
    }

private:
    void render() override
    {
        App::render();
        settings.screenSize = vec2(width_, height_);
        glDisable(GL_DEPTH_TEST);

        ImGui::Begin("Information");
        ImGui::Text("Performance: %.1f FPS (%.2f ms per frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Boid Size", &settings.boidSize, 1.0f, 20.0f);
        ImGui::SliderFloat("Max Speed", &settings.maxSpeed, 0.0f, 1000.0f);
        ImGui::SliderFloat("Max Force", &settings.maxForce, 0.0f, 1000.0f);
        ImGui::SliderFloat("Angle View", &settings.angleView, 0.0f, 360.0f);
        ImGui::SliderFloat("Perception Radius", &settings.perceptionRadius, 0.0f, 100.0f);
        ImGui::SliderFloat("Avoidance Radius", &settings.avoidanceRadius, 0.0f, 100.0f);
        ImGui::SliderFloat("Alignment", &settings.alignmentWeight, 0.0f, 2.0f);
        ImGui::SliderFloat("Cohesion", &settings.cohesionWeight, 0.0f, 2.0f);
        ImGui::SliderFloat("Separation", &settings.separationWeight, 0.0f, 2.0f);
        ImGui::RadioButton("CPU Simulation", &settings.simulationType, 0);
        ImGui::SameLine();
        ImGui::RadioButton("GPU Simulation", &settings.simulationType, 1);
        ImGui::End();

        shader.use();
        shader.set("screenSize", settings.screenSize);
        shader.set("boidSize", settings.boidSize);
        shader.set("maxSpeed", settings.maxSpeed);
        va.bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, writer->id());
        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, settings.amount);
    }

    void update(float deltaTime) override
    {
        simulations[settings.simulationType]->update(deltaTime, &boids, writer.get(), reader.get());
        simulations[settings.simulationType]->sync(&boids, writer.get(), reader.get());
        writer.swap(reader);
    }

    Settings settings;
    GLBuffer model;
    GLVertexArray va;
    vector<Boid> boids;
    unique_ptr<GLBuffer> writer, reader;
    Shader shader;
    vector<unique_ptr<Simulation>> simulations;
};

int main(int argc, char const *argv[])
{
    MyApp app;
    app.run();
    return 0;
}
