#pragma once


namespace Graphics
{
	class Camera
	{
	public:
		virtual glm::mat4 GetViewMatrix() const = 0;
		virtual glm::mat4 GetProjectionMatrix() const = 0;

		void SetSize(uint32_t width, uint32_t height);

	protected:
		uint32_t m_width{ 0 };
		uint32_t m_height{ 0 };
	};

	class Camera2D : public Camera
	{
	public:
		glm::mat4 GetViewMatrix() const override;
		glm::mat4 GetProjectionMatrix() const override;

		auto GetPosition() const { return mPosition; }
		void SetPosition(glm::vec2 value) { mPosition = value; }

	private:
		glm::vec2 mPosition = { 0.0f, 0.0f };
	};

	class Camera3D : public Camera
	{
	public:
		Camera3D();

		void Update();

		void FrontMove(float offset);
		void SideMove(float offset);

		glm::mat4 GetViewMatrix() const override { return mViewMatrix; }
		glm::mat4 GetProjectionMatrix() const override { return mProjectionMatrix; }

		auto GetPosition() const { return mPosition; }
		void SetPosition(glm::vec3 value) { mPosition = value; }

		auto GetYaw() const { return mYaw; }
		void SetYaw(float value) { mYaw = value; }

		auto GetPitch() const { return mPitch; }
		void SetPitch(float value) { mPitch = value; }

		auto GetFieldOfView() const { return mFieldOfView; }
		void SetFieldOfView(float value) { mFieldOfView = value; }

		auto GetWorldUp() const { return mWorldUp; }
		void SetWorldUp(const glm::vec3& value) { mWorldUp = value; }

		auto GetNearPlane() const { return mNearPlane; }
		void SetNearPlane(float value) { mNearPlane = value; }

		auto GetFarPlane() const { return mFarPlane; }
		void SetFarPlane(float value) { mFarPlane = value; }

		auto GetFront() const { return mFront; }
		auto GetUp() const { return mUp; }
		auto GetRight() const { return mRight; }

	private:
		glm::mat4 mViewMatrix = glm::mat4(1.0f);
		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

		glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
		float mYaw = 0.0f;
		float mPitch = 0.0f;
		float mFieldOfView = 70.0f;
		glm::vec3 mWorldUp = { 0.0f, -1.0f, 0.0f };
		float mNearPlane = 1.0f;
		float mFarPlane = 8192.0f;

		glm::vec3 mFront;
		glm::vec3 mUp;
		glm::vec3 mRight;
	};
}