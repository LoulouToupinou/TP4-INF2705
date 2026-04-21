#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "textures.hpp"
#include "shaders.hpp"

#include "happly.h"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;


struct App : public OpenGLApplication
{
    App()
    : cameraPosition_(0.f, 0.f, 0.f)
    , cameraOrientation_(0.f, 0.f)
    , currentScene_(0)
    , isMouseMotionEnabled_(false)
    {
    }

	void init() override
	{
		// Le message expliquant les touches de clavier.
		setKeybindMessage(
			"ESC : quitter l'application." "\n"
			"T : changer de scène." "\n"
			"W : déplacer la caméra vers l'avant." "\n"
			"S : déplacer la caméra vers l'arrière." "\n"
			"A : déplacer la caméra vers la gauche." "\n"
			"D : déplacer la caméra vers la droite." "\n"
			"Q : déplacer la caméra vers le bas." "\n"
			"E : déplacer la caméra vers le haut." "\n"
			"Flèches : tourner la caméra." "\n"
			"Souris : tourner la caméra" "\n"
			"Espace : activer/désactiver la souris." "\n"
		);

		// Config de base.
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Load shaders
        sphereShader_.create();
        phongShader_.create();

        // Load textures
        staffTexture_.load("../textures/Staff.png");
        staffTexture_.setWrap(GL_REPEAT);
        staffTexture_.setFiltering(GL_LINEAR);

        swordTextureBase_.load("../textures/Longsword_10_low_Longsword_10_BaseColor.jpg");
        swordTextureBase_.setWrap(GL_REPEAT);
        swordTextureBase_.setFiltering(GL_LINEAR);

        // Load models
        staffMainModel_.load("../models/staff-main.ply");
        staffSphereModel_.load("../models/staff-sphere.ply");
        swordModel_.load("../models/sword.ply");

	}

	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::SliderFloat("Ambient light", &ambientLight_, 0.0f, 1.0f);
        ImGui::End();
        
        switch (currentScene_)
        {
            case 0: sceneMain();  break;
        }
	}

	// Appelée lorsque la fenêtre se ferme.
	void onClose() override
	{

	}

	// Appelée lors d'une touche de clavier.
	void onKeyPress(const sf::Event::KeyPressed& key) override
	{
		using enum sf::Keyboard::Key;
		switch (key.code)
		{
		    case Escape:
		        window_.close();
	        break;
		    case Space:
		        isMouseMotionEnabled_ = !isMouseMotionEnabled_;
		        if (isMouseMotionEnabled_)
		        {
		            window_.setMouseCursorGrabbed(true);
		            window_.setMouseCursorVisible(false);
	            }
	            else
	            {
	                window_.setMouseCursorGrabbed(false);
	                window_.setMouseCursorVisible(true);
                }
	        break;
	        case T:
                currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
		    default: break;
		}
	}

	void onResize(const sf::Event::Resized& event) override
	{	
	}
	
	void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
	{	    
	    if (!isMouseMotionEnabled_)
	        return;
        
        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
	    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
	}
	
	void updateCameraInput() 
    {
        if (!window_.hasFocus())
            return;
            
        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }
        
        float cameraMouvementX = 0;
        float cameraMouvementY = 0;
        
        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;
        
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;
            
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }
    
    glm::mat4 getViewMatrix()
    {
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(view, -cameraOrientation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, -cameraOrientation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, -cameraPosition_);
        
        return view;
    }
    
    glm::mat4 getPerspectiveProjectionMatrix()
    {
        float fov = glm::radians(70.0f);
        float aspect = getWindowAspect();
        float near = 0.1f;
        float far = 300.0f;
        
        return glm::perspective(fov, aspect, near, far);
    }
    
    void sceneMain()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        updateCameraInput();
        updateAnimations();
        
        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        phongShader_.use();
        glUniform3fv(phongShader_.cameraPositionULoc, 1, glm::value_ptr(cameraPosition_));
        phongShader_.setMaterial({ambientLight_, 0.2f, 1.0f, 32.0f});

        drawStaff(projView);
        drawSword(projView);
    }

    void updateAnimations()
    {
        const float SPHERE_ANIM_SPEED = 0.02f; 
        spherePhase_ += SPHERE_ANIM_SPEED;
        if (spherePhase_ > glm::two_pi<float>()) {
            spherePhase_ -= glm::two_pi<float>();
        }

        // const float SWORD_ROTATION_SPEED = 0.01f;
        // swordAngle_ += SWORD_ROTATION_SPEED;
        
        // if (swordAngle_ > glm::two_pi<float>()) {
        //     swordAngle_ -= glm::two_pi<float>();
        // }
    }

    void drawStaff(const glm::mat4& projView)
    {
        glm::mat4 staffModel = glm::mat4(1.0f);
        staffModel = glm::translate(staffModel, glm::vec3(-0.5f, -2.5f, -2.0f));
        staffModel = glm::rotate(staffModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        staffModel = glm::scale(staffModel, glm::vec3(2.0f));

        const float AMPLITUDE = 0.1f; 
        
        float sphereOffset = std::sin(spherePhase_) * AMPLITUDE;

        glm::mat4 sphereModel = glm::translate(staffModel, glm::vec3(0.0f, sphereOffset, 0.0f));
        glm::mat4 sphereMVP = projView * sphereModel;
        glm::vec3 sphereCenter = glm::vec3(sphereModel * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        phongShader_.use();
        glUniformMatrix4fv(phongShader_.mULoc, 1, GL_FALSE, glm::value_ptr(sphereModel));
        glUniformMatrix4fv(phongShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(sphereMVP));
        glUniform3f(phongShader_.lightPositionULoc, sphereCenter.x, sphereCenter.y, sphereCenter.z);
        glUniform1i(phongShader_.isLightSourceULoc, true);
        glUniform3f(phongShader_.lightColorULoc, 0.1f, 0.85f, 1.0f);
        staffSphereModel_.draw();
        glUniform1i(phongShader_.isLightSourceULoc, false);



        glm::mat4 staffMVP = projView * staffModel;

        staffTexture_.use();
        glUniformMatrix4fv(phongShader_.mULoc, 1, GL_FALSE, glm::value_ptr(staffModel));
        glUniformMatrix4fv(phongShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(staffMVP));
        staffMainModel_.draw();
    }

    void drawSword(const glm::mat4& projView)
    {
        glm::mat4 swordModel = glm::mat4(1.0f);
        swordModel = glm::translate(swordModel, glm::vec3(0.5f, -2.5f, -2.0f));
        swordModel = glm::rotate(swordModel, swordAngle_, glm::vec3(0.0f, 1.0f, 0.0f));
        swordModel = glm::scale(swordModel, glm::vec3(2.0f));

        glm::mat4 swordMVP = projView * swordModel;

        phongShader_.use();
        swordTextureBase_.use();
        glUniformMatrix4fv(phongShader_.mULoc, 1, GL_FALSE, glm::value_ptr(swordModel));
        glUniformMatrix4fv(phongShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(swordMVP));
        swordModel_.draw();
    }
    
private:    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    GLfloat ambientLight_ = 1.0f;
    
    // Imgui var
    const char* const SCENE_NAMES[1] = {
        "Main Scene"
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;
    
    bool isMouseMotionEnabled_;

    Model staffMainModel_;
    Model staffSphereModel_;
    Model swordModel_;

    Texture2D staffTexture_;
    Texture2D swordTextureBase_;

    SphereShader sphereShader_;
    PhongShader phongShader_;

    float sphereOffset_ = 0.0f;      
    float sphereDirection_ = 1.0f;
    float spherePhase_ = 0.0f;
    float swordAngle_ = 0.0f;
};


int main(int argc, char* argv[])
{
	WindowSettings settings = {};
	settings.fps = 60;
	settings.context.depthBits = 24;
	settings.context.stencilBits = 8;
	settings.context.antiAliasingLevel = 4;
	settings.context.majorVersion = 3;
	settings.context.minorVersion = 3;
	settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

	App app;
	app.run(argc, argv, "Tp4", settings);
}
