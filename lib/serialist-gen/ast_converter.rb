require "serialist-gen/utils"

class String
	def is_i?
		!!(self =~ /\A[-+]?[0-9]+\z/)
	end
end

module SerialistGen

	# lowers the ast to a subset of the language
	# performs checks on semantics
	class AstConverter

		include SerialistGen::Utils


		def collapse_expression(expression)
			if expression

				acceptable_keys = []

				expression.each do |key,value|
					if !"#{key}".start_with? "_" or key == :_content
						if value.is_a? Array
							acceptable_keys << key if value.length > 0
						else
							acceptable_keys << key
						end
					end
				end


				acceptable_keys.each do |key|

					if expression[key].is_a? Array
						expression[key].map! { |x| collapse_expression(x) }
					elsif expression[key].is_a? Hash
						expression[key] = collapse_expression(expression[key])
					end
				end


				if acceptable_keys.length == 1

					item = expression[acceptable_keys[0]]

					if item.is_a? Hash and 
						item[:_col] == expression[:_col] and 
						item[:_line] == expression[:_line]

						return item
					elsif item.is_a? Array and
						item.length == 1 and
						item[0][:_col] == expression[:_col] and 
						item[0][:_line] == expression[:_line]

						return item[0]
					end
				end

			end

			expression
		end

		def intrinsic_function_invocation(funcname, params)
			inv = {ref: "#{funcname}"}
			function_invocation inv, params
		end

		def function_invocation(funcname, params)
			# TODO: perform typechecking
			{call: funcname, params: params}
		end

		def convert_unary(expr)

			x = convert_expression(expr[:primary])

			for i in 0...expr[:invocations].length

				if expr[:invocations][i][:_type] == "FunctionInvoke"

					x = function_invocation x,  expr[:invocations][i][:expressions].map {|param| convert_expression(param)}

				elsif expr[:invocations][i][:_type] == "RecordAccess"

					x = intrinsic_function_invocation "acc", [ x, expr[:invocations][i][:membername][:_token] ]

				elsif expr[:invocations][i][:_type] == "ArrayIndex"

					x = intrinsic_function_invocation "get", [ x, convert_expression(expr[:invocations][i][:expression]) ]
					
				end
			end

			x

		end

		def convert_char(expr)
			char = expr[:_token].gsub(/^'/, "").gsub(/'$/, "")
		end

		def convert_expression(expression)
			result = nil
			expr = collapse_expression(expression)

			case expr[:_type]
			when "NumberLit"
				if expr[:_token].is_i?
					result = {int: expr[:_token].to_i} 
				else
					result = {real: expr[:_token].to_f} 
				end
			when "CharLit"
				result = {int: convert_char(expr).ord, char: convert_char(expr)}
			when "ArrayLiteral"
				result = intrinsic_function_invocation "arr", expr[:expressions].map{ |x| convert_expression(x) }
			when "StringLit"
				raise "StringLit conversion not implemented yet"
			when "Unary"
				result = convert_unary(expr)
			when "MemberIdentifier"
				result = {ref: expr[:_token]}
			when "HexLit"
				result = {int: /0x(?<num>[a-f0-9]+)/.match(expr[:_token])[:num].to_i(16)}
			else
				puts "For the following expression:"
				p expr
				raise "handling for #{expr[:_type]} not implemented"
			end

			result

		end

		def get_attributes_named(member, attrname)
			member[:attributes].select{ |att| att[:typeidentifier][:_token] == attrname }
		end

		def attribute_properties
			{
				"ArraySize" => {name: :array_size, limit: 1, params: 1},
				"ConstructWith" => {name: :construct_with, limit: 0, params: 1},
				"Mustcontain" => {name: :must_contain, limit: 1, params: 1, checker: lambda do |member, attribute_data| 

					params = attribute_data[:attributeparams]
					if params.length != 1
						STDERR.puts "line: #{attribute_data[:_line]} col: #{attribute_data[:_col]}"
						STDERR.puts "Attribute MustContain only accepts 1 parameter"
						exit(1)
					end

					check_kind = "unit"
					member_kind = "unit"

					check_kind = "array" if params[0][:expressions][0][:unary] and params[0][:expressions][0][:unary][:_type] == "ArrayLiteral"
					member_kind = "array" if member[:attributes].select {|att| att[:typeidentifier][:_token] == "ArraySize"}.length == 1

					#puts "PARAMS"
					#puts params[0].to_yaml
					#puts "ATTRS"
					#puts member[:attributes].to_yaml

					#puts check_kind
					#p member_kind

					if check_kind != member_kind
						STDERR.puts "line: #{attribute_data[:_line]} col: #{attribute_data[:_col]}"
						STDERR.puts "Attribute MustContain can only receive an Array if the member is an Array"
						exit(1)
					end


				end
				},
				"Parameter" => {name: :is_parameter, limit: 1, params: 0},
				"BigEndian" => {name: :big_endian, limit: 1, params: 0},
				"LittleEndian" => {name: :little_endian, limit: 1, params: 0}
			}
		end

		def get_attribute_enum(member)

			attributes = {}

			member[:attributes].each do |att|
				attributes[att[:typeidentifier][:_token]] = true
			end

			attributes.keys
		end

		def get_attributes(member)

			attributes = {}

			get_attribute_enum(member).each do |att|

				known_attr = attribute_properties[att]

				converted_attrs = get_attributes_named(member, att).map do |attribute_data|
					if attribute_data[:attributeparams].length == 1 #exists

						parameter_list = attribute_data[:attributeparams][0][:expressions].map do |expr|
											convert_expression(expr)
										end

						if known_attr 

							if known_attr[:checker]
								known_attr[:checker].call(member, attribute_data)
							end

							passed_count = attribute_data[:attributeparams][0][:expressions].length

							if passed_count != known_attr[:params]
								STDERR.puts "line: #{attribute_data[:_line]} col: #{attribute_data[:_col]}"
								STDERR.puts "Attribute #{att} needs #{known_attr[:params]} but #{passed_count} were given"
								exit(1)
							end

							if known_attr[:params] == 1
								parameter_list = convert_expression(attribute_data[:attributeparams][0][:expressions].first)
							end

						end

						parameter_list
					end
				end

				converted_name = att

				if known_attr

					if known_attr[:limit] 

						if get_attributes_named(member, att).length > known_attr[:limit] and known_attr[:limit] != 0
							STDERR.puts "line: #{member[:_line]} col: #{member[:_col]}"
							STDERR.puts "There are more than #{known_attr[:limit]} #{att} attributes. Only #{known_attr[:limit]} will be used."
							exit(1)
						end

						if known_attr[:limit] == 1
							converted_attrs = converted_attrs.first
						else
							converted_attrs = converted_attrs[0..known_attr[:limit]]
						end

					end

					converted_name = known_attr[:name]
				else
					# Custom attribute
				end

				attributes[converted_name] = converted_attrs
			end

			attribute_properties

			attributes

		end

		def check_and_convert_member(member)

			converted = {
				type: member[:typeidentifier][:_token],
				name: member[:memberidentifier][:_token],
				attributes: get_attributes(member)
			}

			converted
		end

		def check_and_convert_ast(ast)

			types = {
				"Uint8" => [],
				"Uint16" => [],
				"Uint32" => [],
				"Uint64" => [],
				"Int8" => [],
				"Int16" => [],
				"Int32" => [],
				"Int64" => [],
				"Float32" => [],
				"Float64" => []
			}

			subset_hash = {}

			ast[:elements].
				select {|e| e[:_content][:_type] == "Subset"}.
				map { |element| element[:_content] }.
				each do |subset|

					subset_item = {
						name: subset[:typeidentifiers][0][:_token],
						origin_type: subset[:typeidentifiers][1][:_token],
						elements: subset[:subsetrange][:simpleliterals].map {|x| convert_expression(x)}
					}

					if !simple_type?(subset_item[:origin_type])
						STDERR.puts "line: #{subset[:_line]} col: #{subset[:_col]}"
						STDERR.puts "Subset can only be derived from a basic type"
						exit(1)
					end

					subset_hash[subset[:typeidentifiers][0][:_token]] = subset_item

				end

			result = {
				subsets: subset_hash,

				formats: ast[:elements].
					select {|e| e[:_content][:_type] == "Format"}.
					map { |element| element[:_content] }.
					map do |format|
						f = {
							name: format[:typeidentifier][:_token],
							deptypes: format[:statements].map { |member| member[:typeidentifier][:_token] },

							members: format[:statements].map do |member|
								check_and_convert_member(member)
							end
						}

						types[f[:name]] = f[:deptypes]

						f
					end
			}

			member_ids = {"invalid_member" => 0}

			result[:formats].each do |format|
				format[:members].each do |member|
					member_ids["#{format[:name]}::#{member[:name]}"] = member_ids.length
				end
			end

			result[:member_ids] = member_ids

			result
			
		end

	end
end
