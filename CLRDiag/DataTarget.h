#pragma once

#include <atlcomcli.h>
#include <atlstr.h>
#include <clrdata.h>
#include <unordered_map>
#include <functional>
#include "dacprivate.h"

enum class ManagedTypeKind {
	Class,
	Delegate,
	Enum,
	Interface,
	Struct,
	Array,
	Count
};

enum class ManagedMemberType {
	Method,
	Property,
	Event,
	Field,
	Constructor,
	Type,
	StaticConstructor
};

struct ManagedMember {
	CString Name;
	mdToken Token;
	mdToken ClassToken;
	ManagedMemberType Type;
	DWORD Attributes;
	union {
		struct {
			DWORD TypeFlags;
		} Field;
		struct {
			DWORD ImplFlags;
			DWORD CodeRva;
		} Method;
		struct {
			mdMethodDef AddMethod, RemoveMethod, FireMethod;
			mdToken EventType;
		} Event;
		struct {
			mdToken Setter, Getter;
			DWORD CPlusTypeFlag;
		} Property;
	};
};

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
	CLRDATA_ADDRESS Address;
	DWORD Index;
	CString Name, BaseName;
	ManagedTypeKind Kind;
	DacpMethodTableFieldData FieldData;
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

struct FieldInfo : DacpFieldDescData {
	CString Name;
	ULONG32 Flags;
};

struct MethodInfo : DacpMethodDescData {
	CString Name;
	ULONG32 Flags;
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

enum class GetFieldsFlags {
	None = 0,
	Instance = 1,
	Static = 2,
	ThreadLocal = 4,
	AllNonGenerated = Instance | Static | ThreadLocal,
	CompilerGenerated = 8,
	Inherited = 16,
	Default = AllNonGenerated,

	All = 0xff
};
DEFINE_ENUM_FLAG_OPERATORS(GetFieldsFlags);

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
	ModuleInfo GetModuleInfo(CLRDATA_ADDRESS address);
	std::vector<SyncBlockInfo> EnumSyncBlocks(bool includeFree);
	bool EnumObjects(EnumObjectCallback callback, int heap = -1);
	std::vector<HeapStatItem> GetHeapStats(int heap = -1);
	std::vector<TaskInfo> EnumTasks();
	std::vector<GCHandleInfo> EnumGCHandles();
	std::vector<FieldInfo> GetFieldsOfType(CLRDATA_ADDRESS mt, GetFieldsFlags flags = GetFieldsFlags::Default);
	std::vector<MethodInfo> GetMethodsOfType(CLRDATA_ADDRESS mt);
	std::vector<ManagedMember> EnumTypeMembers(CLRDATA_ADDRESS mt);

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

	template<typename T>
	T ReadVirtual(DWORD64 address) const {
		T value{};
		ULONG32 read;
		_clrTarget->ReadVirtual(address, (BYTE*)&value, sizeof(value), &read);
		return value;
	}

	ULONG32 ReadVirtualBuffer(DWORD64 address, void* buffer, DWORD size) const {
		ULONG32 read;
		_clrTarget->ReadVirtual(address, (BYTE*)buffer, size, &read);
		return read;
	}

	DacpThreadpoolData GetThreadPoolData();
	std::vector<ThreadInfo> EnumThreads(bool includeDeadThreads);
	DacpThreadStoreData GetThreadsStats();
	
	bool IsStringType(CLRDATA_ADDRESS mt) const;
	bool IsFreeType(CLRDATA_ADDRESS mt) const;
	bool IsArrayType(CLRDATA_ADDRESS mt) const;

protected:
	bool EnumMethodTablesInternal(CLRDATA_ADDRESS module, std::vector<MethodTableInfo>& mti);
	bool EnumObjectsInternal(DacpGcHeapDetails& details, EnumObjectCallback callback);
	void EnumModulesInternal(CLRDATA_ADDRESS assembly, std::vector<ModuleInfo>& modules);
	void EnumAssembliesInternal(CLRDATA_ADDRESS appDomain, std::vector<AssemblyInfo>& assemblies);

	virtual HRESULT Init() = 0;
	void DoCommonInit();
	static std::unique_ptr<DataTarget> InitCommon(std::unique_ptr<DataTarget> target);

protected:
	CComPtr<ICLRDataTarget> _clrTarget;
	CComPtr<IUnknown> _spSos;
	std::unordered_map<CLRDATA_ADDRESS, MethodTableInfo> _mtCache;
	CLRDATA_ADDRESS _mtObject{ 0 }, _mtDelegate{ 0 }, _mtEnum{ 0 }, _mtValueType{ 0 };
	DacpUsefulGlobalsData _globals;

};

