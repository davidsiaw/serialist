
module SerialistGen
module Utils extend self

def simple_type?(typename, subsets={})

	if subsets and subsets[typename]
		return true
	end

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

def is_subset?(typename, subsets={})
	subsets[typename] != nil
end

end
end