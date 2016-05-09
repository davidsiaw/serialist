def c_type(typename)
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

def simple_type?(typename)
	case typename
	when "Int8"
		true
	when "Int16"
		true
	when "Int32"
		true
	when "Int64"
		true
	when "Uint8"
		true
	when "Uint16"
		true
	when "Uint32"
		true
	when "Uint64"
		true
	when "Float32"
		true
	when "Float64"
		true
	else
		false
	end
end

def type_length(typename)
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
		"SERIALIST_NULL_FILE_POINTER"
	]
end