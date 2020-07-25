#pragma once

#include <unordered_map>

enum class ManagedTypeKind {
	Attribute,
	Class,
	Delegate,
	Enum,
	Interface,
	Struct,
	Count
};

struct ManagedType {
	CString Name;
	CorTypeAttr Attributes;
	mdTypeDef Token;
	mdToken BaseTypeToken;
	CString BaseTypeName;
	ManagedTypeKind Kind;
};

struct ManagedTypeRef {
	CString Name;
	mdTypeDef Token;
	mdToken Scope;
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

class CLRMetadataParser final {
public:
	explicit CLRMetadataParser(PCWSTR path);
	CLRMetadataParser(PVOID p, DWORD size);
	explicit CLRMetadataParser(IMetaDataImport* pImport);
	bool IsValid() const {
		return _spImport != nullptr;
	}

	std::vector<ManagedType> EnumTypes() const;
	std::vector<ManagedTypeRef> EnumRefTypes() const;
	std::vector<ManagedMember> EnumMembers(mdToken token, bool includeInherited = false) const;
	std::vector<ManagedMember> EnumMethods(mdToken token, bool includeInherited = false) const;
	std::vector<ManagedMember> EnumFields(mdToken token, bool includeInherited = false) const;
	std::vector<ManagedMember> EnumProperties(mdToken token, bool includeInherited = false) const;
	std::vector<ManagedMember> EnumEvents(mdToken token, bool includeInherited = false) const;
	ManagedTypeKind GetTypeKind(const ManagedType& type) const;
	CString GetMethodName(mdMethodDef token) const;
	CString GetTypeName(mdToken token) const;

private:
	void EnumMethodsInternal(mdToken token, std::vector<ManagedMember>& members, bool includeInherited = false) const;
	void EnumFieldsInternal(mdToken token, std::vector<ManagedMember>& members, bool includeInherited = false) const;
	void EnumPropertiesInternal(mdToken token, std::vector<ManagedMember>& members, bool includeInherited = false) const;
	void EnumEventsInternal(mdToken token, std::vector<ManagedMember>& members, bool includeInherited = false) const;

private:
	mutable mdToken _delegate{ 0 }, _enum{ 0 }, _struct{ 0 }, _attribute{ 0 };
	CComPtr<IMetaDataImport> _spImport;
	mutable std::vector<ManagedTypeRef> _refTypes;
	mutable std::unordered_map<mdToken, ManagedTypeRef> _refTypesMap;
};

