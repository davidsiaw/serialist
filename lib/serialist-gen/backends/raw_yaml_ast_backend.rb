
module SerialistGen
module Backends

class RawYamlAstBackend

	def self.desc
		"Generates the raw YAML abstract syntax tree for the format"
	end

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def generate
		<<-ECHOEND
#{@ast.to_yaml}
ECHOEND
	end
end

end
end
