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

struct BezierCurve
{
    glm::vec3 p0;
    glm::vec3 c0;
    glm::vec3 c1;
    glm::vec3 p1;
};

glm::vec3 calculateBezier(const BezierCurve& curve, float t) {
    float u = 1.0f - t;
    const float b0 = std::pow(u, 3);
    const float b1 = 3.0f * t * u * u;
    const float b2 = 3.0f * t * t * u;
    const float b3 = std::pow(t, 3);
    return b0 * curve.p0 + b1 * curve.c0 + b2 * curve.c1 + b3 * curve.p1;
}


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
        bezierShader_.create();
        phongShader_.create();
        bloomShader_.create();
        combineShader_.create();

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

        glGenVertexArrays(1, &vaoBezier_);
        glGenBuffers(1, &vboBezier_);
        glBindVertexArray(vaoBezier_);
        glBindBuffer(GL_ARRAY_BUFFER, vboBezier_);

        std::vector<glm::vec3> bezierPoints;
        const int nSegments = 50;
        for (int j = 0; j <= nSegments; ++j) {
            float t = static_cast<float>(j) / static_cast<float>(nSegments);
            bezierPoints.push_back(calculateBezier(swordCurve_, t));
        }
        numBezierVerts_ = bezierPoints.size();

        glBufferData(GL_ARRAY_BUFFER, numBezierVerts_ * sizeof(glm::vec3), bezierPoints.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);

        glGenFramebuffers(1, &hdrFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
        glGenTextures(2, colorBuffers);
        sf::Vector2u windowSize = window_.getSize();
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
            );
        }  
        GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);

        glGenRenderbuffers(1, &hdrDepthRBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowSize.x, windowSize.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthRBO_);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongBuffers);
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0
            );
        }

        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO_);
        glGenBuffers(1, &quadVBO_);
        glBindVertexArray(quadVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
    }

    void renderQuad()
    {
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        if (ImGui::Button("Animate Sword"))
        {
            isAnimatingSword_ = true;
            swordAnimationProgress_ = 0.0f;
        }
        ImGui::Checkbox("Show Bezier Curve", &showBezierCurve_);
        ImGui::Checkbox("Spin Sword", &isSwordSpinning_);
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
        updateCameraInput();
        updateAnimations();

        // 1. Render scene into HDR FBO (two color attachments: scene + bright parts)
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        glActiveTexture(GL_TEXTURE0);
        phongShader_.use();
        glUniform3fv(phongShader_.cameraPositionULoc, 1, glm::value_ptr(cameraPosition_));
        phongShader_.setMaterial({ambientLight_, 0.2f, 1.0f, 32.0f});

        drawStaff(projView);
        drawSword(projView);

        if (showBezierCurve_)
        {
            drawBezierCurve(projView);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Gaussian blur (ping-pong) on the bright-parts buffer
        int amount = 10;
        bool horizontal = true, first_iteration = true;
        bloomShader_.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            glUniform1i(bloomShader_.horizontalULoc, horizontal);
            glBindTexture(
                GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]
            );
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Combine: blend scene + blurred bloom, apply HDR tone-mapping
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        combineShader_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);
        glUniform1i(combineShader_.sceneULoc, 0);
        glUniform1i(combineShader_.bloomBlurULoc, 1);
        glUniform1f(combineShader_.exposureULoc, exposure_);
        renderQuad();
    }

    void updateAnimations()
    {
        const float SPHERE_ANIM_SPEED = 0.02f; 
        spherePhase_ += SPHERE_ANIM_SPEED;
        if (spherePhase_ > glm::two_pi<float>()) {
            spherePhase_ -= glm::two_pi<float>();
        }

        if (isAnimatingSword_) {
            swordAnimationProgress_ += deltaTime_ * 0.5f; 

            if (swordAnimationProgress_ >= 1.0f) {
                isAnimatingSword_ = false;
                swordAnimationProgress_ = 0.0f;
            }
        } else if (isSwordSpinning_) {
            const float SWORD_ROTATION_SPEED = 0.01f;
            swordAngle_ += SWORD_ROTATION_SPEED;

            if (swordAngle_ > glm::two_pi<float>()) {
                swordAngle_ -= glm::two_pi<float>();
            }
        }
    }

    void drawBezierCurve(const glm::mat4& projView)
    {
        glm::mat4 bezierModel = glm::mat4(1.0f);
        glm::mat4 bezierMVP = projView * bezierModel;

        bezierShader_.use(); 
        glUniformMatrix4fv(bezierShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(bezierMVP));
        glUniform3f(bezierShader_.colorULoc, 1.0f, 0.0f, 0.0f);

        glBindVertexArray(vaoBezier_);
        glDrawArrays(GL_LINE_STRIP, 0, numBezierVerts_);
        glBindVertexArray(0);
    }

    void drawStaff(const glm::mat4& projView)
    {
        const float AMPLITUDE = 0.02f; 
        const float LOCAL_Y_OFFSET = 1.22f;

        glm::mat4 staffModel = glm::mat4(1.0f);
        staffModel = glm::translate(staffModel, glm::vec3(-0.5f, -2.5f, -2.0f));
        staffModel = glm::rotate(staffModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        staffModel = glm::scale(staffModel, glm::vec3(2.0f));
        glm::mat4 staffMVP = projView * staffModel;
        
        float sphereOffset = std::sin(spherePhase_) * AMPLITUDE;

        glm::mat4 sphereModel = glm::translate(staffModel, glm::vec3(0.0f, sphereOffset, 0.0f));
        glm::mat4 sphereMVP = projView * sphereModel;
        glm::vec3 sphereCenter = glm::vec3(sphereModel * glm::vec4(0.0f, LOCAL_Y_OFFSET, 0.0f, 1.0f));

        phongShader_.use();
        glUniformMatrix4fv(phongShader_.mULoc, 1, GL_FALSE, glm::value_ptr(sphereModel));
        glUniformMatrix4fv(phongShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(sphereMVP));
        glUniform3f(phongShader_.lightPositionULoc, sphereCenter.x, sphereCenter.y, sphereCenter.z);
        glUniform1i(phongShader_.isLightSourceULoc, true);
        glUniform3f(phongShader_.lightColorULoc, 0.1f, 0.85f, 1.0f);
        staffSphereModel_.draw();
        glUniform1i(phongShader_.isLightSourceULoc, false);

        staffTexture_.use();
        glUniformMatrix4fv(phongShader_.mULoc, 1, GL_FALSE, glm::value_ptr(staffModel));
        glUniformMatrix4fv(phongShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(staffMVP));
        staffMainModel_.draw();
    }

    void drawSword(const glm::mat4& projView)
    {
        glm::mat4 swordModel = glm::mat4(1.0f);

        if (isAnimatingSword_)
        {
            glm::vec3 currentPos = calculateBezier(swordCurve_, swordAnimationProgress_);
            swordModel = glm::translate(swordModel, currentPos);

            glm::vec3 nextPos = calculateBezier(swordCurve_, swordAnimationProgress_ + 0.01f);
            glm::vec3 dir = nextPos - currentPos;

            dir = glm::normalize(dir);
            float rotY = std::atan2(dir.x, dir.z);
            float rotX = std::atan2(dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));

            swordModel = glm::rotate(swordModel, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
            swordModel = glm::rotate(swordModel, -rotX, glm::vec3(1.0f, 0.0f, 0.0f));

            float tilt = glm::mix(-glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f, swordAnimationProgress_);
            swordModel = glm::rotate(swordModel, tilt, glm::vec3(1.0f, 0.0f, 0.0f));
        } else
        {
            swordModel = glm::translate(swordModel, glm::vec3(0.5f, -0.1774f, -2.0f));
            swordModel = glm::rotate(swordModel, swordAngle_, glm::vec3(0.0f, 1.0f, 0.0f));
            swordModel = glm::rotate(swordModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        swordModel = glm::scale(swordModel, glm::vec3(2.0f));
        swordModel = glm::rotate(swordModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        swordModel = glm::translate(swordModel, glm::vec3(0.0f, -1.1613f, 0.0f));

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

    GLfloat ambientLight_ = 0.5f;
    
    // Imgui var
    const char* const SCENE_NAMES[1] = {
        "Main Scene"
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;
    bool showBezierCurve_ = false;
    
    bool isMouseMotionEnabled_;

    Model staffMainModel_;
    Model staffSphereModel_;
    Model swordModel_;

    Texture2D staffTexture_;
    Texture2D swordTextureBase_;

    BezierShader bezierShader_;
    PhongShader phongShader_;
    BloomShader bloomShader_;

    float sphereOffset_ = 0.0f;      
    float sphereDirection_ = 1.0f;
    float spherePhase_ = 0.0f;
    float swordAngle_ = 0.0f;

    bool isAnimatingSword_ = false;
    float swordAnimationProgress_ = 0.0f;
    bool isSwordSpinning_ = true;
    
    BezierCurve swordCurve_ = {
        glm::vec3(0.5f, -0.1774f, -2.0f),
        glm::vec3(-0.5f, 2.8226f, -2.5f),
        glm::vec3(2.5f, 2.8226f, -2.5f),
        glm::vec3(3.5f, -0.1774f, -2.0f)
    };

    GLuint vaoBezier_ = 0;
    GLuint vboBezier_ = 0;
    int numBezierVerts_ = 0;

    unsigned int hdrFBO_ = 0;
    unsigned int hdrDepthRBO_ = 0;
    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffers[2];
    unsigned int colorBuffers[2];

    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    CombineShader combineShader_;
    float exposure_ = 1.0f;
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
