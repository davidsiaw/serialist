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