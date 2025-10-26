#pragma once

#include "Context.h"
#include "ContextLocator.h"
#include "Core.h"
#include "CoreLocator.h"
#include "IModule.h"

/// GCM (Get Core Module)
/// ���������� ��������� �� ������ ����
#define GCM(type) (&Core::Locator().get<type>())

/// GCXM (Get ConteXt Module)
/// ���������� ��������� �� ������ ���������
#define GCXM(type) (&Context::Locator().get<type>())


using namespace EngineCore::Base;
