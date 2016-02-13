
class String
	def is_i?
		!!(self =~ /\A[-+]?[0-9]+\z/)
	end
end

module SerialistGen


	# lowers the ast to a subset of the language
	# performs checks on semantics
	class AstConverter


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

		def convert_unary(expr)

			x = convert_expression(expr[:primary])

			for i in 0...expr[:invocations].length

				if expr[:invocations][i][:_type] == "FunctionInvoke"
					x = {call: x, params: expr[:invocations][i][:expressions].map {|param| convert_expression(param)} }
				elsif expr[:invocations][i][:_type] == "RecordAccess"
					x = {call: "acc", params: [ x, expr[:invocations][i][:membername][:_token] ] }
				elsif expr[:invocations][i][:_type] == "ArrayIndex"
					x = {call: "get", params: [ x, convert_expression(expr[:invocations][i][:expression]) ] }
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
				"ArraySize" => {name: :array_size, limit: 1, params: 1}
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

						if get_attributes_named(member, att).length > known_attr[:limit]
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

			{
				subsets: ast[:elements].
					select {|e| e[:_content][:_type] == "Subset"}.
					map { |element| element[:_content] }.
					map do |subset|

						{
							name: subset[:typeidentifiers][0][:_token],
							origin_type: subset[:typeidentifiers][1][:_token],
							elements: subset[:subsetrange][:simpleliterals].map {|x| convert_expression(x)}
						}

					end,

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
			
		end

	end
end
