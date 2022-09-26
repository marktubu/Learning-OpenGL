#pragma once

#include <rttr/registration>
using namespace rttr;

class GameObject;
class Component {
public:
	Component();
	virtual ~Component() = default;


public:
	GameObject* gameObject;

public:
	virtual void Awake();
	virtual void OnEnable();
	virtual void Update();
	virtual void FixedUpdate();
	virtual void OnDisable();

	virtual void OnPreRender();
	virtual void OnPostRender();


private:
	
RTTR_ENABLE();
};