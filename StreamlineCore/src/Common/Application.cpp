#include "Application.h"

#include "Graphics/Renderer.h"
#include "ImGui/Widgets.h"

namespace slc {

	Application::Application(ApplicationSpecification* spec)
		: IEventListener(EventManager::ListenerType::App), mSpecification(spec)
	{
		if (sInstance)
		{
			ASSERT(false, "Application already exists");
			return;
		}
		sInstance = this;

		if (!spec->workingDir.empty())
			std::filesystem::current_path(spec->workingDir);

		mWindow = Window::Create(WindowProperties(spec->name, spec->resolution, spec->fullscreen));

		mImGuiController = ImGuiController::Create(mWindow->GetNativeWindow());
	}

	Application::~Application()
	{
		for (ApplicationLayer* layer : mLayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		mImGuiController.reset();
		mWindow.reset();

		delete mSpecification;

		for (const auto& shutdownTask : mAppSystems | std::views::reverse)
			shutdownTask();
	}

	void Application::OnEvent(Event& e)
	{
		e.Dispatch<WindowCloseEvent>(SLC_BIND_EVENT_FUNC(OnWindowClose));
		e.Dispatch<WindowResizeEvent>(SLC_BIND_EVENT_FUNC(OnWindowResize));
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Application::Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.width == 0 || e.height == 0)
		{
			mState.minimised = true;
			return false;
		}

		mState.minimised = false;
		Renderer::SetViewport(e.width, e.height);
		return false;
	}

	void Application::ExecuteMainThread()
	{
		std::scoped_lock<std::mutex> lock(sInstance->mState.mainThreadQueueMutex);

		for (auto& func : sInstance->mState.mainThreadQueue)
			func();

		sInstance->mState.mainThreadQueue.clear();
	}

	void Application::Run(int argc, char** argv)
	{
		Application* app = CreateApplication(argc, argv);
		if (sInstance != app)
		{
			delete app;
			ASSERT(false, "There was already an app instance, could not create a new one");
			return;
		}

		while (sInstance->mState.running)
		{
			float time = Timestep::Now();
			Timestep timestep = time - sInstance->mState.lastFrameTime;
			sInstance->mState.lastFrameTime = time;

			// Process any queued tasks that could not be performed within main loop.
			sInstance->ExecuteMainThread();

			// Process any events in the event queue
			EventManager::Dispatch();

			// Run update method for each frame
			if (!sInstance->mState.minimised)
			{
				for (auto* layer : sInstance->mLayerStack)
					layer->OnUpdate(timestep);
			}

			// Begin ImGui rendering
			sInstance->mImGuiController->StartFrame();

			// Render each ImGui controls in each layer
			for (ApplicationLayer* layer : sInstance->mLayerStack)
				layer->OnRender();

			// End ImGui rendering
			sInstance->mImGuiController->EndFrame();

			// Poll GLFW events to populate queue and swap buffers
			sInstance->mWindow->OnUpdate();
		}

		delete sInstance;
	}

	void Application::Close()
	{
		if (!sInstance->mState.blockExit) 
			sInstance->mState.running = false;
	}

	void Application::BlockEsc(bool block)
	{
		sInstance->mState.blockExit = block;
	}

	void Application::BlockEvents(bool block)
	{
		sInstance->mImGuiController->BlockEvents(block);
	}
}