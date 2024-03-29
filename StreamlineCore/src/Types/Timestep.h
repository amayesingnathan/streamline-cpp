#pragma once

namespace slc {

	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			: mTime(time)
		{
		}

		operator float() const { return mTime; }

		float getSeconds() const { return mTime; }
		float getMilliseconds() const { return mTime * 1000.f; }

	private:
		float mTime;

	public:
		static float Now();
	};

}