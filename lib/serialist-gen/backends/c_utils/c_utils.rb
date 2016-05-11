require "serialist-gen/utils"

module SerialistGen
module Backends
module CUtils extend self

include SerialistGen::Utils

def c_type(typename, subsets={})

	if subsets and subsets[typename]
		typename = subsets[typename][:origin_type]
	end

	case typename
	when "Int8"
		"int8_t"
	when "Int16"
		"int16_t"
	when "Int32"
		"int32_t"
	when "Int64"
		"int64_t"
	when "Uint8"
		"uint8_t"
	when "Uint16"
		"uint16_t"
	when "Uint32"
		"uint32_t"
	when "Uint64"
		"uint64_t"
	when "Float32"
		"float"
	when "Float64"
		"double"
	else
		"#{typename}*"
	end
end

def type_length(typename, subsets={})

	if subsets and subsets[typename]
		typename = subsets[typename][:origin_type]
	end

	case typename
	when "Int8"
		1
	when "Int16"
		2
	when "Int32"
		4
	when "Int64"
		8
	when "Uint8"
		1
	when "Uint16"
		2
	when "Uint32"
		4
	when "Uint64"
		8
	when "Float32"
		4
	when "Float64"
		8
	else
		0
	end
end

def error_types
	[
		"SERIALIST_NO_ERROR",
		"SERIALIST_OUT_OF_MEMORY",
		"SERIALIST_INDEX_OUT_OF_BOUNDS",
		"SERIALIST_UNEXPECTED_END_OF_FILE",
		"SERIALIST_NULL_POINTER",
		"SERIALIST_WRITE_FAILED",
		"SERIALIST_POINTER_IS_WRONG_TYPE",
		"SERIALIST_NULL_FILE_POINTER",
		"SERIALIST_MEMBER_VALUE_WRONG"
	]
end

end
end
end