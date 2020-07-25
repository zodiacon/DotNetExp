#include "pch.h"
#include "DataTarget.h"
#include "LiveProcessDataTarget.h"
#include "DumpFileDataTarget.h"
#include "dacprivate.h"

inline static size_t Align(ULONG64 nbytes) {
#ifdef _WIN64
	return (nbytes + 7) & ~7;
#else
	return (nbytes + 3) & ~3;
#endif
}

std::unique_ptr<DataTarget> DataTarget::FromProcessId(DWORD pid) {
	std::unique_ptr<DataTarget> target = std::make_unique<LiveProcessDataTarget>(pid);
	return target->Init() == S_OK ? std::move(target) : nullptr;
}

std::unique_ptr<DataTarget> DataTarget::FromDumpFile(PCWSTR dumpFilePath) {
	std::unique_ptr<DataTarget>  target = std::make_unique<DumpFileDataTarget>(dumpFilePath);
	return target->Init() == S_OK ? std::move(target) : nullptr;
}

std::vector<AppDomainInfo> DataTarget::EnumAppDomains(bool includeSharedSys) {
	std::vector<AppDomainInfo> domains;

	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	CLRDATA_ADDRESS addr[64];
	unsigned needed;
	auto hr = spSos->GetAppDomainList(_countof(addr), addr, &needed);
	if (FAILED(hr))
		return domains;

	domains.reserve(needed);
	for (DWORD i = 0; i < needed; i++) {
		domains.push_back(GetAppDomainInfo(addr[i]));
	}
	if (includeSharedSys) {
		DacpAppDomainStoreData data;
		spSos->GetAppDomainStoreData(&data);
		AppDomainInfo info = GetAppDomainInfo(data.sharedDomain);
		info.Name = L"Shared Domain";
		domains.push_back(std::move(info));
		info = GetAppDomainInfo(data.systemDomain);
		info.Name = L"System Domain";
		domains.push_back(std::move(info));
	}

	return domains;
}

std::vector<AssemblyInfo> DataTarget::EnumAssemblies(AppDomainInfo& ad) {
	return EnumAssemblies(ad.AppDomainPtr);
}

std::vector<ModuleInfo> DataTarget::EnumModules(const DacpAssemblyData& assembly) {
	return EnumModules(assembly.AssemblyPtr);
}

std::vector<ModuleInfo> DataTarget::EnumModules(CLRDATA_ADDRESS assembly) {
	std::vector<ModuleInfo> modules;
	EnumModulesInternal(assembly, modules);
	return modules;
}

std::vector<ModuleInfo> DataTarget::EnumModulesInAppDomain(const DacpAppDomainData& ad) {
	return EnumModulesInAppDomain(ad.AppDomainPtr);
}

void DataTarget::EnumModulesInternal(CLRDATA_ADDRESS assembly, std::vector<ModuleInfo>& modules) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	CLRDATA_ADDRESS addr[16];
	unsigned count;
	auto hr = spSos->GetAssemblyModuleList(assembly, _countof(addr), addr, &count);
	if (FAILED(hr))
		return;

	modules.reserve(count);
	for (DWORD i = 0; i < count; i++) {
		ModuleInfo data;
		spSos->GetModuleData(addr[i], &data);
		CComPtr<IXCLRDataModule> spModule;
		spSos->GetModule(addr[i], &spModule);
		if (spModule) {
			WCHAR name[MAX_PATH];
			ULONG32 len;
			if (S_OK == spModule->GetName(_countof(name), &len, name))
				data.Name = name;
			if (S_OK == spModule->GetFileName(_countof(name), &len, name))
				data.FileName = name;
		}
		modules.push_back(data);
	}
}

void DataTarget::EnumAssembliesInternal(CLRDATA_ADDRESS appDomain, std::vector<AssemblyInfo>& assemblies) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	CLRDATA_ADDRESS addr[512];
	int count;
	if (FAILED(spSos->GetAssemblyList(appDomain, _countof(addr), addr, &count)))
		return;

	WCHAR name[512];
	unsigned len;
	for (int i = 0; i < count; i++) {
		AssemblyInfo data;
		if (FAILED(spSos->GetAssemblyData(appDomain, addr[i], &data)))
			continue;
		spSos->GetAssemblyName(addr[i], _countof(name), name, &len);
		data.Name = name;

		assemblies.push_back(std::move(data));
	}
}

std::vector<ModuleInfo> DataTarget::EnumModulesInAppDomain(CLRDATA_ADDRESS addr) {
	std::vector<ModuleInfo> modules;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	CLRDATA_ADDRESS assemblies[512];
	int count;
	if (FAILED(spSos->GetAssemblyList(addr, _countof(assemblies), assemblies, &count)))
		return modules;

	modules.reserve(count);
	for (int i = 0; i < count; i++) {
		EnumModulesInternal(assemblies[i], modules);
	}
	return modules;
}

std::vector<ModuleInfo> DataTarget::EnumModules() {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	std::vector<ModuleInfo> modules;
	CLRDATA_ADDRESS addr[16];
	unsigned needed;
	auto hr = spSos->GetAppDomainList(_countof(addr), addr, &needed);
	if (FAILED(hr))
		return modules;

	int count;
	CLRDATA_ADDRESS assemblies[512];
	for (DWORD i = 0; i < needed; i++) {
		spSos->GetAssemblyList(addr[i], _countof(assemblies), assemblies, &count);
		for (int j = 0; j < count; j++)
			EnumModulesInternal(assemblies[j], modules);
	}
	return modules;
}

ModuleInfo DataTarget::GetModuleInfo(CLRDATA_ADDRESS address) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	ModuleInfo info;
	spSos->GetModuleData(address, &info);
	CComPtr<IXCLRDataModule> spModule;
	spSos->GetModule(address, &spModule);
	return info;
}

std::vector<SyncBlockInfo> DataTarget::EnumSyncBlocks(bool includeFree) {
	std::vector<SyncBlockInfo> sbs;

	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	SyncBlockInfo data;
	auto hr = spSos->GetSyncBlockData(1, &data);
	if (FAILED(hr))	// no sync blocks
		return sbs;

	sbs.reserve(64);
	if (includeFree || !data.bFree) {
		data.Index = 1;
		sbs.push_back(data);
	}
	auto count = data.SyncBlockCount;
	for (UINT i = 2; i <= count; i++) {
		spSos->GetSyncBlockData(i, &data);
		if (includeFree || !data.bFree) {
			data.Index = i;
			sbs.push_back(data);
		}
	}

	return sbs;
}

DacpThreadData DataTarget::GetThreadData(CLRDATA_ADDRESS addr) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpThreadData data;
	spSos->GetThreadData(addr, &data);
	return data;
}

AppDomainInfo DataTarget::GetSharedDomain() {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpAppDomainStoreData data;
	auto hr = spSos->GetAppDomainStoreData(&data);
	ATLASSERT(SUCCEEDED(hr));
	return GetAppDomainInfo(data.sharedDomain);
}

AppDomainInfo DataTarget::GetSystemDomain() {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpAppDomainStoreData data;
	spSos->GetAppDomainStoreData(&data);
	return GetAppDomainInfo(data.systemDomain);
}

int DataTarget::GetAppDomainCount() const {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpAppDomainStoreData data;
	spSos->GetAppDomainStoreData(&data);
	return data.DomainCount;
}

AppDomainInfo DataTarget::GetAppDomainInfo(CLRDATA_ADDRESS addr) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	AppDomainInfo data;
	WCHAR name[MAX_PATH];
	unsigned len;
	spSos->GetAppDomainData(addr, &data);
	spSos->GetAppDomainName(addr, _countof(name), name, &len);
	data.Name = name;
	return data;
}

std::vector<MethodTableInfo> DataTarget::EnumMethodTables(CLRDATA_ADDRESS module) {
	std::vector<MethodTableInfo> mts;
	EnumMethodTablesInternal(module, mts);
	return mts;
}

std::vector<MethodTableInfo> DataTarget::EnumMethodTables() {
	std::vector<MethodTableInfo> mts;
	for (auto& m : EnumModules()) {
		EnumMethodTablesInternal(m.Address, mts);
	}
	return mts;
}

DacpGcHeapData DataTarget::GetGCInfo() const {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpGcHeapData info;
	spSos->GetGCHeapData(&info);
	return info;
}

DacpGcHeapDetails DataTarget::GetWksHeap() {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpGcHeapDetails details;
	spSos->GetGCHeapStaticData(&details);
	return details;
}

bool DataTarget::EnumObjects(EnumObjectCallback callback) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpGcHeapData data;
	spSos->GetGCHeapData(&data);
	CLRDATA_ADDRESS heaps[256];
	unsigned count;
	DacpGcHeapDetails details;
	HRESULT hr;
	if (data.bServerMode) {
		hr = spSos->GetGCHeapList(_countof(heaps), heaps, &count);
		if (FAILED(hr))
			return false;
		for (unsigned i = 0; i < count; i++) {
			hr = spSos->GetGCHeapDetails(heaps[i], &details);
			ATLASSERT(SUCCEEDED(hr));
			if (!EnumObjectsInternal(details, callback))
				break;
		}
	}
	else {
		hr = spSos->GetGCHeapStaticData(&details);
		if (FAILED(hr))
			return false;
		EnumObjectsInternal(details, callback);
	}
	return true;
}

std::vector<AssemblyInfo> DataTarget::EnumAssemblies(CLRDATA_ADDRESS appDomainAddress) {
	std::vector<AssemblyInfo> assemblies;

	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	CLRDATA_ADDRESS addr[1024];
	int count;
	auto hr = spSos->GetAssemblyList(appDomainAddress, _countof(addr), addr, &count);
	if (FAILED(hr))
		return assemblies;

	EnumAssembliesInternal(appDomainAddress, assemblies);

	return assemblies;
}

std::vector<AssemblyInfo> DataTarget::EnumAssemblies(bool includeSysSharedDomains) {
	std::vector<AssemblyInfo> assemblies;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	CLRDATA_ADDRESS addr[64];
	unsigned needed;
	auto hr = spSos->GetAppDomainList(_countof(addr), addr, &needed);
	if (FAILED(hr))
		return assemblies;
	assemblies.reserve(64);
	for (DWORD i = 0; i < needed; i++) {
		EnumAssembliesInternal(addr[i], assemblies);
	}
	if (includeSysSharedDomains) {
		DacpAppDomainStoreData data;
		spSos->GetAppDomainStoreData(&data);
		EnumAssembliesInternal(data.systemDomain, assemblies);
	}
	return assemblies;
}

DacpThreadpoolData DataTarget::GetThreadPoolData() {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	DacpThreadpoolData data;
	spSos->GetThreadpoolData(&data);
	return data;
}

std::vector<ThreadInfo> DataTarget::EnumThreads(bool includeDeadThreads) {
	std::vector<ThreadInfo> threads;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	ATLASSERT(spSos);
	CComQIPtr<IXCLRDataProcess2> spProcess(_spSos);
	ATLASSERT(spProcess);

	auto stat = GetThreadsStats();
	threads.reserve(stat.threadCount + 10);
	for (auto addr = stat.firstThread; addr; ) {
		ThreadInfo data;
		spSos->GetThreadData(addr, &data);
		if (includeDeadThreads || data.osThreadId > 0) {
			spSos->GetStackLimits(addr, &data.StackLow, &data.StackHigh, &data.StackCurrent);
			threads.emplace_back(data);
		}
		addr = data.nextThread;
	}
	return threads;
}

DacpThreadStoreData DataTarget::GetThreadsStats() {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	ATLASSERT(spSos);

	DacpThreadStoreData stat;
	spSos->GetThreadStoreData(&stat);

	return stat;
}

constexpr int min_obj_size = sizeof(BYTE*) + sizeof(PVOID) + sizeof(size_t);

bool DataTarget::EnumMethodTablesInternal(CLRDATA_ADDRESS module, std::vector<MethodTableInfo>& mts) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	std::vector<std::pair<ULONGLONG, DWORD>> addr;
	addr.reserve(1024);
	spSos->TraverseModuleMap(TYPEDEFTOMETHODTABLE, module, [](UINT index, CLRDATA_ADDRESS mt, PVOID param) {
		auto vec = (std::vector<std::pair<ULONGLONG, DWORD>>*)param;
		vec->push_back({ mt, index });
		}, &addr);

	mts.reserve(mts.size() + addr.size());
	WCHAR name[512];
	unsigned count;
	for (auto& [mt, index] : addr) {
		auto it = _mtCache.find(mt);
		if (it == _mtCache.end()) {
			MethodTableInfo data;
			spSos->GetMethodTableData(mt, &data);
			data.Index = index;
			if (S_OK == spSos->GetMethodTableName(mt, _countof(name), name, &count))
				data.Name = name;
			if (!data.bIsFree) {
				if (data.ParentMethodTable) {
					if (S_OK == spSos->GetMethodTableName(data.ParentMethodTable, _countof(name), name, &count))
						data.BaseName = name;
				}
				spSos->GetMethodTableFieldData(mt, &data.FieldData);
				if (_mtObject == 0 && data.Name == L"System.Object")
					_mtObject = mt;
				else if (_mtDelegate == 0 && data.Name == L"System.MulticastDelegate")
					_mtDelegate = mt;
				else if (_mtEnum == 0 && data.Name == L"System.Enum")
					_mtEnum = mt;
				else if (_mtValueType == 0 && data.Name == L"System.ValueType")
					_mtValueType = mt;

				if (data.dwAttrClass & tdInterface)
					data.Kind = ManagedTypeKind::Interface;
				else if (data.ParentMethodTable == _mtEnum || data.BaseName == L"System.Enum")
					data.Kind = ManagedTypeKind::Enum;
				else if (data.ParentMethodTable == _mtDelegate || data.BaseName == L"System.MulticastDelegate")
					data.Kind = ManagedTypeKind::Delegate;
				else if (data.ParentMethodTable == _mtValueType || data.BaseName == L"System.ValueType")
					data.Kind = ManagedTypeKind::Struct;
				else
					data.Kind = ManagedTypeKind::Class;
				_mtCache.insert({ mt, data });
			}
			mts.push_back(data);
		}
		else {
			mts.push_back(it->second);
		}
	}
	return true;
}

bool DataTarget::EnumObjectsInternal(DacpGcHeapDetails& heap, EnumObjectCallback callback) {
	DacpHeapSegmentData segdata;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	ObjectInfo obj;
	WCHAR text[256];

	int gen = 0;
	auto gen0start = heap.generation_table[0].allocation_start;
	auto gen0end = heap.alloc_allocated;
	auto segment = heap.generation_table[2].start_segment;
	auto currentSeg = segment;
	auto hr = segdata.Request(spSos, segment, heap);
	ATLASSERT(SUCCEEDED(hr));

	auto current = segdata.mem;
	bool prevFree = false;

	for (;;) {
		auto segmentEnd = heap.alloc_allocated;
		if (currentSeg == heap.ephemeral_heap_segment) {
			if (current - sizeof(PVOID) == gen0end - Align(min_obj_size))
				break;
		}
		if (current >= segmentEnd) {
			if (current > segmentEnd)
				break;

			segment = segdata.next;
			if (segment) {
				if (segdata.Request(spSos, segment, heap) != S_OK)
					break;
				current = segdata.mem;
				currentSeg = segment;
			}
			else
				break;
		}

		if (currentSeg == heap.ephemeral_heap_segment && current >= gen0end) {
			// something is wrong
			break;
		}
		hr = spSos->GetObjectData(current, &obj);
		if (FAILED(hr))
			break;

		obj.Address = current;
		obj.Generation = gen;
		if (!callback(obj))
			return true;
		current += Align(obj.Size);
	}

	// LOH

	segment = heap.generation_table[3].start_segment;
	currentSeg = segment;
	if (segdata.Request(spSos, segment, heap) != S_OK)
		return false;

	current = segdata.mem;
	for (;;) {
		auto segmentEnd = segdata.used;
		if (current >= segmentEnd) {
			if (current > segmentEnd)
				break;

			segment = segdata.next;
			if (segment) {
				if (segdata.Request(spSos, segment, heap) != S_OK)
					break;
				current = segdata.mem;
				currentSeg = segment;
			}
			else
				break;
		}

		hr = spSos->GetObjectData(current, &obj);
		if (FAILED(hr))
			break;

		if (obj.ObjectType == OBJ_STRING) {
			unsigned len;
			hr = spSos->GetObjectStringData(current, _countof(text), text, &len);
			ATLASSERT(SUCCEEDED(hr));
			obj.StringValue = text;
		}
		obj.Address = current;
		obj.Generation = 3;
		if (!callback(obj))
			return true;
		current += Align(obj.Size);
	}
	return true;
}

CString DataTarget::GetObjectClassName(CLRDATA_ADDRESS address) {
	WCHAR className[512];
	unsigned len;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	auto hr = spSos->GetObjectClassName(address, _countof(className), className, &len);
	return hr == S_OK ? className : L"";
}

CString DataTarget::GetObjectString(CLRDATA_ADDRESS address, unsigned maxLength) {
	auto buffer = std::make_unique<WCHAR[]>(maxLength);
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	unsigned len;
	auto hr = spSos->GetObjectStringData(address, maxLength, buffer.get(), &len);
	return FAILED(hr) ? L"" : buffer.get();
}

MethodTableInfo DataTarget::GetMethodTableInfo(CLRDATA_ADDRESS mt) {
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	MethodTableInfo info;
	spSos->GetMethodTableData(mt, &info);
	WCHAR name[512];
	unsigned len;
	spSos->GetMethodTableName(mt, _countof(name), name, &len);
	info.Name = name;
	return info;
}

std::vector<HeapStatItem> DataTarget::GetHeapStats(CLRDATA_ADDRESS address) {
	std::unordered_map<CLRDATA_ADDRESS, HeapStatItem> items;
	items.reserve(1024);

	WCHAR name[512];
	HRESULT hr;
	unsigned len;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	EnumObjects([&](ObjectInfo& obj) {
		auto it = items.find(obj.MethodTable);
		if (it == items.end()) {
			HeapStatItem info;
			info.MethodTable = obj.MethodTable;
			info.ObjectCount = 1;
			info.TotalSize = obj.Size;
			info.Type = obj.ObjectType;
			if (obj.ObjectType != OBJ_FREE) {
				hr = spSos->GetObjectClassName(obj.Address, _countof(name), name, &len);
				if (hr == S_OK)
					info.TypeName = name;
			}
			else
				info.TypeName = L"*Free*";
			items.insert({ info.MethodTable, info });
		}
		else {
			it->second.TotalSize += obj.Size;
			it->second.ObjectCount++;
		}
		return true;
		});
	std::vector<HeapStatItem> hitems;
	hitems.reserve(items.size());
	for (auto& [addr, mt] : items)
		hitems.push_back(mt);

	return hitems;
}

std::vector<TaskInfo> DataTarget::EnumTasks() {
	CComQIPtr<IXCLRDataProcess> spProcess(_spSos);
	ATLASSERT(spProcess);
	CLRDATA_ENUM hEnum;
	std::vector<TaskInfo> tasks;

	WCHAR name[256];
	ULONG32 len;
	if (S_OK == spProcess->StartEnumTasks(&hEnum)) {
		tasks.reserve(8);
		for (;;) {
			CComPtr<IXCLRDataTask> spTask;
			spProcess->EnumTask(&hEnum, &spTask);
			if (spTask == nullptr)
				break;
			TaskInfo info;
			spTask->GetUniqueID(&info.Id);
			spTask->GetOSThreadID(&info.OSThreadId);
			if (S_OK == spTask->GetName(_countof(name), &len, name))
				info.Name = name;
			CComPtr<IXCLRDataValue> spObject;
			spTask->GetManagedObject(&spObject);
			ATLASSERT(spObject);
			spObject->GetAddress(&info.ObjectAddress);
			tasks.push_back(info);
		}
		spProcess->EndEnumTasks(hEnum);
	}
	return tasks;
}

std::vector<GCHandleInfo> DataTarget::EnumGCHandles() {
	std::vector<GCHandleInfo> handles;
	CComQIPtr<ISOSDacInterface> spSos(_spSos);
	ATLASSERT(spSos);
	CComPtr<ISOSHandleEnum> spEnum;
	GCHandleType types[] = { GCHandleType::Strong, GCHandleType::WeakShort };
	spSos->GetHandleEnumForTypes((unsigned*)types, _countof(types), &spEnum);
	if (!spEnum)
		return handles;

	unsigned count = 0;
	auto hr = spEnum->GetCount(&count);
	if (FAILED(hr))
		return handles;

	handles.reserve(count);
	spEnum->Next(count, handles.data(), &count);

	return handles;
}

TaskInfo DataTarget::GetTaskById(ULONG64 id) {
	TaskInfo info = { 0 };
	CComQIPtr<IXCLRDataProcess> spProcess(_spSos);
	CComPtr<IXCLRDataTask> spTask;
	spProcess->GetTaskByUniqueID(id, &spTask);
	if (spTask == nullptr)
		return info;

	spTask->GetUniqueID(&info.Id);
	spTask->GetOSThreadID(&info.OSThreadId);
	CComPtr<IXCLRDataValue> spObject;
	spTask->GetManagedObject(&spObject);
	if (spObject)
		spObject->GetAddress(&info.ObjectAddress);

	return info;
}