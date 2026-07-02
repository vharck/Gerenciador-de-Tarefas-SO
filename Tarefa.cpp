#include "../include/cpu.hpp"
#include "../include/execucao.hpp"

using namespace ProjetoSO;

CPU::CPU(int id)
{
    this->id = id;
    this->t = nullptr;
    this->ocupado = false;
    this->tempo_desligado = 0;
    this->tempo_exec = 0;
}

int CPU::getId() const { return id; }

Tarefa *CPU::getTarefa() const { return t; }

bool CPU::isOcupado() const { return ocupado; }

int CPU::getTempo_desligado() const { return tempo_desligado; }

int CPU::getTempo_exec() const { return tempo_exec; }

void CPU::setTarefa(Tarefa *nova_tarefa)
{
    this->t = nova_tarefa;
    this->ocupado = (nova_tarefa != nullptr);
    events.push_back({(unsigned int) events.size() + 1, Execucao::getInstance()->getRelogio(), nova_tarefa});
}

void CPU::liberarCPU()
{
    this->t = nullptr;
    this->ocupado = false;
    events.push_back({(unsigned int) events.size() + 1, Execucao::getInstance()->getRelogio(), nullptr});
}

void CPU::unidadeDesligado()
{
    if (!ocupado)
    {
        tempo_desligado++;
    }
}

void CPU::executando()
{
    tempo_exec++;
}

void CPU::resetExec()
{
    tempo_exec = 0;
}