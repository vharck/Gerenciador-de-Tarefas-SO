#pragma once
#include <string>
#include "raylib.h"
#include <vector>
#include "tarefa.hpp"

namespace ProjetoSO
{
	class Interface
	{
	public:
		enum class InterfaceState
		{
			Waiting,
			Simulation,
			Paused,
			Results
		};
	protected:
		

		enum class SimulationType
		{
			SRTF,
			PRIOP,
			FCFS,
			SJF,
			PRIOPENV,
			PRIOD,
			RR
		};


		const unsigned int screenWidth;
		const unsigned int screenHeight;
		const char *title;

		char inputStart[9] = "";
		char inputDuration[9] = "";
		char inputPriority[9] = "";
		char inputQuantum[9] = "";
		char inputCPUcount[9] = "";
		char currentTarget = 0; // 0: nenhum, 1: start, 2: duration, 3: priority, 4: quantum, 5: cpu count

		int alpha;

		SimulationType simulationType = SimulationType::SRTF;
		bool showingDropbox = false;
		const int opcount = 7;

		InterfaceState currentState = InterfaceState::Waiting;
		std::vector<Tarefa *> tasks;

		Tarefa *selectedTask = nullptr;
		
		unsigned int scrollX = 0;
		unsigned int scrollY = 0;
		unsigned int scrollSpeed = 30;

		unsigned int currentStep = 0;

		void drawTextfield(unsigned int x, unsigned int y, char *input, unsigned int id);
		bool drawButton(unsigned int x, unsigned int y, const char *text, Color color, unsigned int width = 80);
		bool drawDropbox(unsigned int x, unsigned int y);
		void drawTasks();
		bool drawTask(unsigned int priority, unsigned int x, unsigned int y, unsigned int taskWidth, Color color);
		void drawBoard();
		void drawGantt();
		void drawWaiting();
		bool drawSimulation();
		void inputHandler();
		void addTask();
		void initSimulation();
		void exportResults();

	public:
		Interface(unsigned int width, unsigned int height, const char *title);
		virtual ~Interface() = default;
		bool draw();
		void simulationFinished();

		std::string getSimTypeText(SimulationType type) const;

		std::vector<Tarefa*> getTasks() const { return tasks; }
		void setTasks(const std::vector<Tarefa *> &newTasks) { tasks = newTasks; }
		void setQuantum(unsigned int q) { snprintf(inputQuantum, sizeof(inputQuantum), "%d", q); }
		void setCPUCount(unsigned int c) { snprintf(inputCPUcount, sizeof(inputCPUcount), "%d", c); }
		void setSimulationType(std::string type);
		void setAlpha(int alpha){this->alpha = alpha;}
	};
}