#pragma once

#include <atlcomcli.h>
#include <atlstr.h>
#include <clrdata.h>
#include <unordered_map>
#include <functional>
#include "dacprivate.h"

struct AppDomainInfo : DacpAppDomainData {
	CString Name;
};

struct ModuleInfo : DacpModuleData {
	CString Name, FileName;
};

struct AssemblyInfo : DacpAssemblyData {
	CString Name;
};

struct MethodTableInfo : DacpMethodTableData {
	DWORD Index;
	CString Name;
};

struct SyncBlockInfo : DacpSyncBlockData {
	int Index;
};

enum class ThreadType {
	Unknown = 0,
	ThreadPoolWorker,
	Finalizer,
};

struct TaskInfo {
	ULONG64 Id;
	ULONG32 OSThreadId;
	CLRDATA_ADDRESS ObjectAddress;
	CString Name;
};

struct ThreadInfo : DacpThreadData {
	ULONGLONG StackLow;
	ULONGLONG StackHigh;
	ULONGLONG StackCurrent;
	ThreadType Type;
};

struct ObjectInfo : DacpObjectData {
	CLRDATA_ADDRESS Address;
	int Generation;
	CString StringValue;
};

struct HeapStatItem {
	CLRDATA_ADDRESS MethodTable;
	unsigned ObjectCount;
	long long TotalSize;
	DacpObjectType Type;
	CString TypeName;
};

struct FieldInfo {
	CString Name;
};

struct MethodInfo {
	CString Name;
};

struct TypeInfo : DacpMethodTableData {
	CString Name;
};

enum class GCHandleType {
	WeakShort = 0,
	WeakLong = 1,
	Strong = 2,
	Pinned = 3,
	RefCounted = 5,
	Dependent = 6,
	AsyncPinned = 7,
	SizedRef = 8,
	WeakWinRT = 9,
};

struct GCHandleInfo : SOSHandleData {
};

using EnumObjectCallback = std::function<bool(ObjectInfo& obj)>;

class DataTarget abstract {
public:
	static std::unique_ptr<DataTarget> FromProcessId(DWORD pid);
	static std::unique_ptr<DataTarget> FromDumpFile(PCWSTR dumpFilePath);

	virtual DWORD GetProcessId() const = 0;
	virtual CString GetProcessPathName() = 0;
	virtual FILETIME GetProcessStartTime() = 0;
	virtual bool IsDump() const = 0;

	virtual bool Suspend() {
		return false;
	}
	virtual bool Resume() {
		return false;
	}

	std::vector<AppDomainInfo> EnumAppDomains(bool includeSharedSys = true);
	std::vector<AssemblyInfo> EnumAssemblies(AppDomainInfo& ad);
	std::vector<AssemblyInfo> EnumAssemblies(CLRDATA_ADDRESS appDomainAddress);
	std::vector<AssemblyInfo> EnumAssemblies(bool includeSysSharedDomains = false);
	std::vector<ModuleInfo> EnumModules(const DacpAssemblyData& assembly);
	std::vector<ModuleInfo> EnumModules(CLRDATA_ADDRESS assembly);
	std::vector<ModuleInfo> EnumModulesInAppDomain(const DacpAppDomainData& ad);
	std::vector<ModuleInfo> EnumModulesInAppDomain(CLRDATA_ADDRESS addr);
	std::vector<ModuleInfo> EnumModules();
	std::vector<SyncBlockInfo> EnumSyncBlocks(bool includeFree);
	bool EnumObjects(EnumObjectCallback callback);
	std::vector<HeapStatItem> GetHeapStats(CLRDATA_ADDRESS address = 0);
	std::vector<TaskInfo> EnumTasks();
	std::vector<GCHandleInfo> EnumGCHandles();

	DacpThreadData GetThreadData(CLRDATA_ADDRESS addr);
	CString GetObjectClassName(CLRDATA_ADDRESS address);
	CString GetObjectString(CLRDATA_ADDRESS address, unsigned maxLength = 256);
	MethodTableInfo GetMethodTableInfo(CLRDATA_ADDRESS mt);
	TaskInfo GetTaskById(ULONG64 id);

	AppDomainInfo GetSharedDomain();
	AppDomainInfo GetSystemDomain();
	int GetAppDomainCount() const;
	AppDomainInfo GetAppDomainInfo(CLRDATA_ADDRESS addr);
	std::vector<MethodTableInfo> EnumMethodTables(CLRDATA_ADDRESS module);
	std::vector<MethodTableInfo> EnumMethodTables();

	DacpGcHeapData GetGCInfo() const;

	DacpGcHeapDetails GetWksHeap();

	DacpThreadpoolData GetThreadPoolData();
	std::vector<ThreadInfo> EnumThreads(bool includeDeadThreads);
	DacpThreadStoreData GetThreadsStats();
	
protected:
	bool EnumMethodTablesInternal(CLRDATA_ADDRESS module, std::vector<MethodTableInfo>& mti);
	bool EnumObjectsInternal(DacpGcHeapDetails& details, EnumObjectCallback callback);
	void EnumModulesInternal(CLRDATA_ADDRESS assembly, std::vector<ModuleInfo>& modules);
	void EnumAssembliesInternal(CLRDATA_ADDRESS appDomain, std::vector<AssemblyInfo>& assemblies);

	virtual HRESULT Init() = 0;

protected:
	CComPtr<ICLRDataTarget> _clrTarget;
	CComPtr<IUnknown> _spSos;
	std::unordered_map<CLRDATA_ADDRESS, MethodTableInfo> _mtCache;
};

