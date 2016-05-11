
require "serialist-gen/backends/c_utils/c_backend_base"

module SerialistGen
module Backends

class CSourceBackend < CUtils::CBackendBase

	def self.desc
		"Generates the C sources for the format"
	end


	def convert_intrinsic_function(expr)
		parmArray = expr[:params].map{|param| generate_expression(param)}
		parms = parmArray.join(", ")
		if expr[:call][:ref] != nil
			func_name = expr[:call][:ref]
			case func_name
			when "add"
				"(#{parmArray[0]} + #{parmArray[1]})"
			when "sub"
				"(#{parmArray[0]} - #{parmArray[1]})"
			when "mul"
				"(#{parmArray[0]} * #{parmArray[1]})"
			when "div"
				"(#{parmArray[0]} / #{parmArray[1]})"

			when "and"
				"(#{parmArray[0]} & #{parmArray[1]})"
			when "or"
				"(#{parmArray[0]} | #{parmArray[1]})"
			when "xor"
				"(#{parmArray[0]} ^ #{parmArray[1]})"

			when "eq"
				"(#{parmArray[0]} == #{parmArray[1]})"
			when "neq"
				"(#{parmArray[0]} != #{parmArray[1]})"

			when "lt"
				"(#{parmArray[0]} < #{parmArray[1]})"
			when "lte"
				"(#{parmArray[0]} <= #{parmArray[1]})"
			when "gt"
				"(#{parmArray[0]} > #{parmArray[1]})"
			when "gte"
				"(#{parmArray[0]} >= #{parmArray[1]})"

			when "andb"
				"(#{parmArray[0]} && #{parmArray[1]})"
			when "orb"
				"(#{parmArray[0]} || #{parmArray[1]})"

			when "sl"
				"(#{parmArray[0]} << #{parmArray[1]})"
			when "sr"
				"(#{parmArray[0]} >> #{parmArray[1]})"

			when "if"
				"(#{parmArray[0]} ? #{parmArray[1]} : #{parmArray[2]})"
			when "acc"
				name = parmArray[0]
				member_name = parmArray[1].sub("pointer->", "")
				"#{name}->#{member_name}"
			when "get"
				name = parmArray[0]
				index = parmArray[1]
				"#{name}[#{index}]"

			when "arr"
				"(0 /* create_array(#{parmArray.map{|x| "#{x}"}.join(", ")}) */)"

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
		elsif expr[:real] != nil
			"#{expr[:real]}"
		else
			puts "The expression:"
			p expr
			raise "Expression unknown!"
		end
	end

	def generate_endian_conversion_code(member)

		endian_decls = ""
		endian_code = ""

		if member[:attributes].has_key? :big_endian

			length = type_length(member[:type])

			endian_decls = <<-ENDIANDECLS
	#{c_type(member[:type])} #{(0...length).map{|x| "b#{x}"}.join(",")};
			ENDIANDECLS

			case length
			when 2
				endian_code = <<-ENDIANCODE
	b0 = (member & 0x00ff) << 8u;
	b1 = (member & 0xff00) >> 8u;
	member = b0 | b1;
				ENDIANCODE
			when 4
				endian_code = <<-ENDIANCODE
	b0 = (member & 0x000000ff) << 24u;
	b1 = (member & 0x0000ff00) << 8u;
	b2 = (member & 0x00ff0000) >> 8u;
	b3 = (member & 0xff000000) >> 24u;
	member = b0 | b1 | b2 | b3;
				ENDIANCODE
			when 8
				endian_code = <<-ENDIANCODE
	b0 = (member & 0x00000000000000ff) << 56ul;
	b1 = (member & 0x000000000000ff00) << 40ul;
	b2 = (member & 0x0000000000ff0000) << 24ul;
	b3 = (member & 0x00000000ff000000) << 8ul;
	b4 = (member & 0x000000ff00000000) >> 8ul;
	b5 = (member & 0x0000ff0000000000) >> 24ul;
	b6 = (member & 0x00ff000000000000) >> 40ul;
	b7 = (member & 0xff00000000000000) >> 56ul;
	member = b0 | b1 | b2 | b3 | b4 | b5 | b6 | b7;
				ENDIANCODE
			when 0
				raise "Not simple type but trying to write endian code"
			end
		end

		{decls: endian_decls, code: endian_code}
	end

	def generate_member_readers(format,members)
		members.map do |member|

			reader = ""
			reader_decls = ""
			endian_decls = ""

			if member[:attributes].has_key? :is_parameter
				""
			else

				if simple_type? member[:type]

					endian_conv_code = generate_endian_conversion_code(member)

					endian_code = endian_conv_code[:code]
					endian_decls = endian_conv_code[:decls]

					reader = <<-READER_CODE
	amt_read = fread(&member, sizeof(#{c_type(member[:type])}), 1, fp);
#{endian_code}
	if (amt_read != 1)
	{
		return SERIALIST_UNEXPECTED_END_OF_FILE;
	}
					READER_CODE

					reader_decls = <<-READER_DECLS
	size_t amt_read = 0;
					READER_DECLS

				else
					parameters_pass = ""
					if member[:attributes][:construct_with]
						member[:attributes][:construct_with].map do |parm|
							parameters_pass += ", #{generate_expression(parm)}"
						end
					end

					reader = <<-READER_CODE
	error_code = Read#{member[:type]}(fp, &member#{parameters_pass});
	if (error_code)
	{
		return error_code;
	}
					READER_CODE
				end

				value_checker = ""

				if member[:attributes].has_key? :array_size

					value_checker = <<-CHECKER
		error_code = Set#{format[:name]}_#{member[:name]}(pointer, member, index);
					CHECKER

					if member[:attributes][:must_contain]

						value_checker = ""

						count = 0

						member[:attributes][:must_contain][:params].each do |value|
							must_contain_expression = generate_expression(value)

							value_checker += <<-CHECKER
		if (index == #{count} && member != #{must_contain_expression})
		{
			return SERIALIST_MEMBER_VALUE_WRONG;
		}
							CHECKER
							count += 1
						end
					end


					<<-MEMBERREAD
SerialistError Read#{format[:name]}_#{member[:name]}(FILE* fp, #{format[:name]}* pointer)
{
	#{c_type(member[:type])} member;
#{reader_decls}
#{endian_decls}
	size_t index = 0;
	size_t count = 0;
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	error_code = Count#{format[:name]}_#{member[:name]}(pointer, &count);
	if (error_code)
	{
		return error_code;
	}

	for (index = 0; index < count; index++)
	{
#{reader}
#{value_checker}
		if (error_code)
		{
			return error_code;
		}
	}
	return SERIALIST_NO_ERROR;
}
					MEMBERREAD
				else
					value_checker = <<-CHECKER
	error_code = Set#{format[:name]}_#{member[:name]}(pointer, member);
					CHECKER

					if member[:attributes][:must_contain]
						must_contain_expression = generate_expression(member[:attributes][:must_contain])
						value_checker = <<-CHECKER
	if (member != #{must_contain_expression})
	{
		return SERIALIST_MEMBER_VALUE_WRONG;
	}
						CHECKER
					end

					<<-MEMBERREAD
SerialistError Read#{format[:name]}_#{member[:name]}(FILE* fp, #{format[:name]}* pointer)
{
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	#{c_type(member[:type])} member;
#{reader_decls}
#{endian_decls}
#{reader}
#{value_checker}
	if (error_code)
	{
		return error_code;
	}
	return SERIALIST_NO_ERROR;
}
					MEMBERREAD
				end
			end


		end.join ""
	end

	def generate_member_read_calls(format,members)
		members.map do |member|

			if member[:attributes].has_key? :is_parameter
				<<-MEMBERREAD
	/* #{member[:name]} is a parameter */
				MEMBERREAD
			else
				<<-MEMBERREAD
	error_code = Read#{format[:name]}_#{member[:name]}(fp, pointer);
	if (error_code) { goto error; }
				MEMBERREAD
			end

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

	def generate_member_writers(format, members)

		members.map do |member|
			if member[:attributes].has_key? :array_size
				<<-MEMBERWRITE
SerialistError Write#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, FILE* fp)
{
	#{c_type(member[:type])} item;
	size_t index = 0;
	size_t count = 0;
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

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
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	error_code = Get#{format[:name]}_#{member[:name]}(pointer, &item);
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


	def generate_member_functions(format, members)

		members.map do |member|
			subset_checker = "1"

			if is_subset?(member[:type])
				subset_checker = "IsValueElementOf#{member[:type]}(value)"
			end

			if member[:attributes].has_key? :array_size

				setter = <<-MEMBERSETTER

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
		if (pointer->SIZEOF_#{member[:name]} < expected_size)
		{
			/* The array size increased. We need to zero out the new memory */
			memset(&pointer->#{member[:name]}[pointer->SIZEOF_#{member[:name]}], 0, (expected_size - pointer->SIZEOF_#{member[:name]}) * sizeof(#{c_type(member[:type])}) ) ;
		}
		pointer->SIZEOF_#{member[:name]} = expected_size;
	}
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

	if (#{subset_checker} == 0)
	{
		return SERIALIST_INVALID_VALUE_FOR_TYPE;
	}
	
	pointer->#{member[:name]}[index] = value;

	return SERIALIST_NO_ERROR;
}
				MEMBERSETTER

				count_returner = <<-RETURNER
	*out_count = #{generate_expression(member[:attributes][:array_size])};
				RETURNER

				value_returner = <<-RETURNER
	*out_item = pointer->#{member[:name]}[index];
				RETURNER

				size_of_expression = "pointer->SIZEOF_#{member[:name]}"
				if member[:attributes][:must_contain]
					setter = ""
					size_of_expression = member[:attributes][:must_contain][:params].length

					count = 0

					case_expressions = ""
					member[:attributes][:must_contain][:params].each do |value|
						value_at_expression = generate_expression(value)

						case_expressions += <<-RETURNER
		case #{count}:
			*out_item = #{value_at_expression};
			break;
						RETURNER
						count += 1
					end

					count_returner = <<-RETURNER
	*out_count = #{member[:attributes][:must_contain][:params].length};
					RETURNER

					value_returner = <<-RETURNER
	switch(index)
	{
#{case_expressions}
		default:
			return SERIALIST_INDEX_OUT_OF_BOUNDS;
	}
					RETURNER

				end

				<<-ENDMEMBERPROTO

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
#{count_returner}
	return SERIALIST_NO_ERROR;
}

SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t index, #{c_type(member[:type])}* out_item)
{
	SerialistError error_code = SERIALIST_NO_ERROR;
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
	if (index >= #{size_of_expression})
	{
		return SERIALIST_INDEX_OUT_OF_BOUNDS;
	}
#{value_returner}
	return SERIALIST_NO_ERROR;
}
#{setter}
				ENDMEMBERPROTO
			else

			get_statement = <<-GETSTATEMENT
	*out_item = pointer->#{member[:name]};
			GETSTATEMENT

			setter = <<-MEMBERSETTER
SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (!#{subset_checker})
	{
		return SERIALIST_INVALID_VALUE_FOR_TYPE;
	}

	pointer->#{member[:name]} = value;
	
	return SERIALIST_NO_ERROR;
}
			MEMBERSETTER

			if member[:attributes][:must_contain]
				setter = ""
				get_statement = <<-GETSTATEMENT
	*out_item = #{generate_expression(member[:attributes][:must_contain])};
				GETSTATEMENT
			end

			<<-ENDMEMBERPROTO
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
#{get_statement}
	return SERIALIST_NO_ERROR;
}

#{setter}
			ENDMEMBERPROTO


			end
		end.join ""
	end


	def generate_functions
		@ast[:formats].map do |format|

			initializers = ""

			format[:members].each do |member|
				if member[:attributes][:construct_with]
					if simple_type?(member[:type])
						initializers += <<-MEMBERINITIALIZER
	Set#{format[:name]}_#{member[:name]}(pointer, #{generate_expression(member[:attributes][:construct_with].first)});
						MEMBERINITIALIZER
					end
				end
			end

			parameters_decl = format[:members].select do |member|
				member[:attributes].has_key? :is_parameter
			end.map do |member|
				", #{c_type(member[:type])} in_#{member[:name]}"
			end.join ""

			parameters_pass = format[:members].select do |member|
				member[:attributes].has_key? :is_parameter
			end.map do |member|
				", in_#{member[:name]}"
			end.join ""

			parameters_assign = format[:members].select do |member|
				member[:attributes].has_key? :is_parameter
			end.map do |member|
				<<-ASSIGNMENT
	pointer->#{member[:name]} = in_#{member[:name]};
				ASSIGNMENT
			end.join ""


			<<-ENDSTRUCT
SerialistError Create#{format[:name]} (#{format[:name]}** out_pointer#{parameters_decl})
{
	#{format[:name]}* pointer = (#{format[:name]}*)calloc(1, sizeof(#{format[:name]}));
	if (pointer == NULL)
	{
		return SERIALIST_OUT_OF_MEMORY;
	}
	if (out_pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	pointer->TYPE_SIGNATURE = #{format[:name].upcase}_TYPE;
#{initializers}
#{parameters_assign}

	*out_pointer = pointer;
	return SERIALIST_NO_ERROR;
}

SerialistError Delete#{format[:name]} (#{format[:name]}* pointer)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	if (!IsPointerOfType(pointer, #{format[:name].upcase}_TYPE))
	{
		return SERIALIST_POINTER_IS_WRONG_TYPE;
	}

	free(pointer);
	return SERIALIST_NO_ERROR;
}

#{generate_member_readers(format, format[:members])}
#{generate_member_writers(format, format[:members])}
SerialistError Read#{format[:name]} (FILE* fp, #{format[:name]}** out_pointer#{parameters_decl})
{
	SerialistError error_code = SERIALIST_NO_ERROR;
	#{format[:name]}* pointer;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	if (out_pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	error_code = Create#{format[:name]}(&pointer#{parameters_pass});
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

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	if (!IsPointerOfType(pointer, #{format[:name].upcase}_TYPE))
	{
		return SERIALIST_POINTER_IS_WRONG_TYPE;
	}

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

			if member[:attributes][:must_contain]
				""
			else
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
			end


		end.join ""
	end

	def generate_structures
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
struct #{format[:name]}
{
	#{@name.capitalize}Types TYPE_SIGNATURE;
#{generate_structure_members(format)}
};
			ENDSTRUCT
		end.join ""
	end

	def generate_subset_checkers
		@ast[:subsets].map do |name, subset|

			switch_ladder = subset[:elements].map do |elem|
				<<-CASE
		case #{generate_expression(elem)}:
			return 1;
				CASE
			end.join ""

			<<-CHECKING_FUNCTION
static int IsValueElementOf#{name}(#{c_type(subset[:origin_type])} value)
{
	switch(value)
	{
#{switch_ladder}
	}
	return 0;
}
			CHECKING_FUNCTION
		end.join ""
	end

	def generate

		error_list = ""
		werror_list = ""

		for i in 0...error_types.length
			line = <<-LINE
		case #{error_types[i]}:
			return "#{error_types[i]}";
			LINE
			error_list << line
			werror_list << line.sub(' "', ' L"')

		end

		<<-CHEADEREND
#include "#{@name.downcase}.h"

#{generate_subset_checkers}

static int IsPointerOfType(void* pointer, #{@name.capitalize}Types type)
{
	if (pointer == NULL)
	{
		return 0;
	}
	if( *(#{@name.capitalize}Types*)pointer == type )
	{
		return 1;
	}
	return 0;
}

const char* SerialistErrorString(SerialistError err)
{
	switch(err)
	{
#{error_list}
	}

	return "SERIALIST_UNKNOWN_ERROR";
}

const wchar_t* SerialistErrorWString(SerialistError err)
{
	switch(err)
	{
#{werror_list}
	}

	return L"SERIALIST_UNKNOWN_ERROR";
}

#{generate_structures}
#{generate_functions}

CHEADEREND
	end
end

end
end