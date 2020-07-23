#include "stdafx.h"
#include "Target.h"

Target::Target(std::unique_ptr<DataTarget> dt) : _dataTarget(std::move(dt)) {
}

Target::~Target() = default;

DataTarget* Target::GetDataTarget() {
	return _dataTarget.get();
}
