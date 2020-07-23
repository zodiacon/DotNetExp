#pragma once

#include "DataTarget.h"

class Target {
public:
	Target(std::unique_ptr<DataTarget> dt);
	Target(Target&&) = default;
	~Target();

	DataTarget* GetDataTarget();

private:
	std::unique_ptr<DataTarget> _dataTarget;
};

