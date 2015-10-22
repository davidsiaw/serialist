
module SerialistGen
module Backends

class RawJsonAstBackend

	def self.desc
		"Generates the raw JSON abstract syntax tree for the format"
	end

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def generate
		<<-ECHOEND
#{@ast.to_json}
ECHOEND
	end
end

end
end