
module SerialistGen
module Backends

require "serialist-gen/backends/c_utils/c_utils"

class CSourceBackend

	def self.desc
		"Generates the C sources for the format"
	end

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def convert_intrinsic_function(expr)
		parmArray = expr[:params].map{|param| generate_expression(param)}
		parms = parmArray.join(", ")
		if expr[:call][:ref] != nil
			func_name = expr[:call][:ref]
			case func_name
			when "add"
				"(#{parmArray[0]} + #{parmArray[1]})"
			else
				"#{func_name}(#{parms})"
			end
		else
			"#{generate_expression(expr[:call])}(#{parms})"
		end
	end

	def generate_expression(expr)
		if expr[:int] != nil
			"#{expr[:int]}"
		elsif expr[:char] != nil
			"'#{expr[:char]}'"
		elsif expr[:call] != nil
			convert_intrinsic_function(expr)
		elsif expr[:ref] != nil
			"pointer->#{expr[:ref]}"
		end
	end

	def generate_member_readers(format,members)
		members.map do |member|

			if member[:attributes].has_key? :array_size
				<<-MEMBERREAD
SerialistError Read#{format[:name]}_#{member[:name]}(FILE* fp, #{format[:name]}* pointer)
{
	#{c_type(member[:type])} member;
	size_t amt_read = 0;
	size_t index = 0;
	size_t count = 0;
	SerialistError error_code = SERIALIST_NO_ERROR;
	error_code = Count#{format[:name]}_#{member[:name]}(pointer, &count);
	if (error_code)
	{
		return error_code;
	}

	for (index = 0; index < count; index++)
	{
		amt_read = fread(&member, sizeof(#{c_type(member[:type])}), 1, fp);
		if (amt_read != sizeof(#{c_type(member[:type])}))
		{
			return SERIALIST_UNEXPECTED_END_OF_FILE;
		}
		error_code = Set#{format[:name]}_#{member[:name]}(pointer, member, index);
		if (error_code)
		{
			return error_code;
		}
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERREAD
			else
				<<-MEMBERREAD
SerialistError Read#{format[:name]}_#{member[:name]}(FILE* fp, #{format[:name]}* pointer)
{
	SerialistError error_code = SERIALIST_NO_ERROR;
	#{c_type(member[:type])} member;
	size_t amt_read = fread(&member, sizeof(#{c_type(member[:type])}), 1, fp);
	if (amt_read != sizeof(#{c_type(member[:type])}))
	{
		return SERIALIST_UNEXPECTED_END_OF_FILE;
	}
	error_code = Set#{format[:name]}_#{member[:name]}(pointer, member);
	if (error_code)
	{
		return error_code;
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERREAD
			end

		end.join ""
	end

	def generate_member_read_calls(format,members)
		members.map do |member|
			<<-MEMBERREAD
	error_code = Read#{format[:name]}_#{member[:name]}(fp, pointer);
	if (error_code) { goto error; }
			MEMBERREAD
		end.join ""
	end

	def generate_member_write_calls(format,members)
		members.map do |member|
			<<-MEMBERREAD
	error_code = Write#{format[:name]}_#{member[:name]}(pointer, fp);
	if (error_code) { goto error; }
			MEMBERREAD
		end.join ""
	end

	def generate_member_writers(format,members)
		members.map do |member|
			if member[:attributes].has_key? :array_size
				<<-MEMBERWRITE
SerialistError Write#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, FILE* fp)
{
	#{c_type(member[:type])} item;
	size_t index = 0;
	size_t count = 0;
	SerialistError error_code = SERIALIST_NO_ERROR;
	error_code = Count#{format[:name]}_#{member[:name]}(pointer, &count);
	size_t amt_written = 0;

	for (index = 0; index < count; index++)
	{
		Get#{format[:name]}_#{member[:name]}(pointer, index, &item);
		if (error_code)
		{
			return error_code;
		}
		fwrite(&item, sizeof(#{c_type(member[:type])}), 1, fp);
		if (amt_written != sizeof(#{c_type(member[:type])}))
		{
			return SERIALIST_WRITE_FAILED;
		}
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERWRITE
			else
				<<-MEMBERWRITE
SerialistError Write#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, FILE* fp)
{
	#{c_type(member[:type])} item;
	SerialistError error_code = Get#{format[:name]}_#{member[:name]}(pointer, &item);
	size_t amt_written = 0;
	if (error_code)
	{
		return error_code;
	}
	fwrite(&item, sizeof(#{c_type(member[:type])}), 1, fp);
	if (amt_written != sizeof(#{c_type(member[:type])}))
	{
		return SERIALIST_WRITE_FAILED;
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERWRITE
			end
		end.join ""
	end


	def generate_member_functions(format,members)
		members.map do |member|
			if member[:attributes].has_key? :array_size
			<<-ENDMEMBERPROTO
SerialistError Check#{format[:name]}_#{member[:name]}ArraySize(#{format[:name]}* pointer)
{
	/* Method for lazily updating the array size, so that if it is dependant on some other
	 * variable, it can be quickly checked
	 */
	size_t expected_size = 0;
	SerialistError error_code = Count#{format[:name]}_#{member[:name]}(pointer, &expected_size);
	if (error_code)
	{
		return error_code;
	}
	if (pointer->SIZEOF_#{member[:name]} != expected_size)
	{
		pointer->#{member[:name]} = realloc(pointer->#{member[:name]}, expected_size * sizeof(#{c_type(member[:type])}));
		if (pointer->#{member[:name]} == NULL)
		{
			return SERIALIST_OUT_OF_MEMORY;
		}
		pointer->SIZEOF_#{member[:name]} = expected_size;
	}
	return SERIALIST_NO_ERROR;
}

SerialistError Count#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t* out_count)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (out_count == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	*out_count = #{generate_expression(member[:attributes][:array_size])};

	return SERIALIST_NO_ERROR;
}

SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value, size_t index)
{
	SerialistError error_code = Check#{format[:name]}_#{member[:name]}ArraySize(pointer);
	if (error_code)
	{
		return error_code;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (index >= pointer->SIZEOF_#{member[:name]})
	{
		return SERIALIST_INDEX_OUT_OF_BOUNDS;
	}

	pointer->#{member[:name]}[index] = value;
	return SERIALIST_NO_ERROR;
}

SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t index, #{c_type(member[:type])}* out_item)
{
	SerialistError error_code = Check#{format[:name]}_#{member[:name]}ArraySize(pointer);
	if (error_code)
	{
		return error_code;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (out_item == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (index >= pointer->SIZEOF_#{member[:name]})
	{
		return SERIALIST_INDEX_OUT_OF_BOUNDS;
	}
	*out_item = pointer->#{member[:name]}[index];
	return SERIALIST_NO_ERROR;
}
			ENDMEMBERPROTO
			else
			<<-ENDMEMBERPROTO
SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value)
{
	pointer->#{member[:name]} = value;
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	return SERIALIST_NO_ERROR;
}

SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])}* out_item)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (out_item == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	*out_item = pointer->#{member[:name]};
	return SERIALIST_NO_ERROR;
}
			ENDMEMBERPROTO
			end
		end.join ""
	end

	def generate_functions
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
SerialistError Create#{format[:name]} (#{format[:name]}** out_pointer)
{
	#{format[:name]}* new_object = (#{format[:name]}*)malloc(sizeof(#{format[:name]}));
	if (new_object == NULL)
	{
		return SERIALIST_OUT_OF_MEMORY;
	}
	if (out_pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	*out_pointer = new_object;
	return SERIALIST_NO_ERROR;
}

SerialistError Delete#{format[:name]} (#{format[:name]}* pointer)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	free(pointer);
	return SERIALIST_NO_ERROR;
}

#{generate_member_readers(format, format[:members])}
#{generate_member_writers(format, format[:members])}
SerialistError Read#{format[:name]} (FILE* fp, #{format[:name]}** out_pointer)
{
	SerialistError error_code = SERIALIST_NO_ERROR;
	#{format[:name]}* pointer;

	if (out_pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	error_code = Create#{format[:name]}(&pointer);
	if (error_code) { goto error; }
#{generate_member_read_calls(format, format[:members])}
	*out_pointer = pointer;
	return error_code;

error:
	Delete#{format[:name]}(pointer);
	return error_code;
}

SerialistError Write#{format[:name]} (#{format[:name]}* pointer, FILE* fp)
{
	SerialistError error_code = SERIALIST_NO_ERROR;
#{generate_member_write_calls(format, format[:members])}

error:
	return error_code;
}

#{generate_member_functions(format, format[:members])}
			ENDSTRUCT
		end.join "\n"
	end

	def generate_structure_members(format)
		format[:members].map do |member|

			if member[:attributes].has_key? :array_size
				<<-ENDMEMBER
	#{c_type(member[:type])}* #{member[:name]};
	size_t SIZEOF_#{member[:name]};
				ENDMEMBER
			else
				<<-ENDMEMBER
	#{c_type(member[:type])} #{member[:name]};
				ENDMEMBER
			end

		end.join ""
	end

	def generate_structures
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
struct #{format[:name]}
{
#{generate_structure_members(format)}
};
			ENDSTRUCT
		end.join ""
	end

	def generate

		<<-CHEADEREND
#include "#{@name.downcase}.h"

#{generate_structures}
#{generate_functions}

CHEADEREND
	end
end

end
end