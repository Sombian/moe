import re, ssl, os.path, urllib.request

URL = "https://www.unicode.org/Public/UNIDATA" # unicode.org
ssl._create_default_https_context = ssl._create_unverified_context

ORG = {}

def GET(name: str):
	# cache miss
	if name not in ORG:
		with urllib.request.urlopen(f"{URL}/{name}") as response:
			ORG[name] = response.read().decode("utf-8").splitlines()
	# cache hit
	return ORG[name]

#|-----------------|
#| data extractors |
#|-----------------|

def UnicodeData(name: str): 
	out = set()

	for line in GET("UnicodeData.txt"):
		pass

	# print(out)
	return out

def CaseFolding(name: str):
	out = set()

	for line in GET("CaseFolding.txt"):
		pass

	# print(out)
	return out

def CompositionExclusions(name: str):
	out = set()

	for line in GET("CompositionExclusions.txt"):
		pass

	# print(out)
	return out

def DerivedCoreProperties(name: str):
	out = set()
	# match A (value)
	R1 = re.compile(r"^([0-9A-F]+) *; *(\w+)")
	# match A..B (range)
	R2 = re.compile(r"^([0-9A-F]+)\.\.([0-9A-F]+) *; *(\w+)")

	for line in GET("DerivedCoreProperties.txt"):
		# reset
		m = None
		l = None
		h = None
		a = None

		# try R1
		if m := R1.match(line):
			# extract
			l = m.group(1)
			h = m.group(1)
			a = m.group(2)

		# try R2
		if m := R2.match(line):
			# extract
			l = m.group(1)
			h = m.group(2)
			a = m.group(3)

		# filter
		if a == name:
			# extract
			out.update(range(int(l, 16), int(h, 16) + 1))

	# print(out)
	return out

def GraphemeBreakProperty(name: str):
	out = set()

	for line in GET("auxiliary/GraphemeBreakProperty.txt"):
		pass

	# print(out)
	return out

#|--------------------|
#| multi-stage tables |
#|--------------------|

class Props:
	def __init__(self):
		self.xid_start = "false"
		self.xid_continue = "false"

		# self.bidi_mirror = None
		# self.decompose_type = None

		# self.bidi_class = None
		# self.combining_class = None

		# self.decompose_mapping = None
		# self.lowercase_mapping = None
		# self.uppercase_mapping = None
		# self.titlecase_mapping = None

	def __str__(self):
		return f"props {{{self.xid_start}, {self.xid_continue}}}"

BLOCK_SIZE = 128

class Generator:
	@staticmethod
	def build(data):

		stage1 = []
		stage2 = []
		stage3 = []
		stage3_map = {}  # Props string -> index in stage3
		stage2_map = {}  # Tuple of 128 ints -> block start index in stage2

		block = []

		for code in range(0x10FFFF + 1):
			# Get the property object or default
			prop = data.get(code, Props())
			prop_key = str(prop)

			# Map to stage3 index (create new if needed)
			if prop_key not in stage3_map:
				stage3_map[prop_key] = len(stage3)
				stage3.append(prop)
			stage3_index = stage3_map[prop_key]

			# Add index to current block
			block.append(stage3_index)

			# When block fills (128 entries)
			if len(block) == BLOCK_SIZE:
				block_key = tuple(block)

				# Check for existing block in stage2
				if block_key not in stage2_map:
					stage2_map[block_key] = len(stage2)
					stage2.extend(block)  # Append block to stage2

				block_start = stage2_map[block_key]
				stage1.append(block_start)
				block = []

		# Handle partial final block if Unicode count isn't divisible by BLOCK_SIZE
		if block:
			# Pad block with zeros (pointing to default prop)
			default_prop = Props()
			default_index = stage3_map.get(str(default_prop), 0)
			while len(block) < BLOCK_SIZE:
				block.append(default_index)

			block_key = tuple(block)
			if block_key not in stage2_map:
				stage2_map[block_key] = len(stage2)
				stage2.extend(block)

			block_start = stage2_map[block_key]
			stage1.append(block_start)

		return stage1, stage2, stage3

#|------------------------|
#| final output (3 files) |
#|------------------------|

OUT = {}

for code in DerivedCoreProperties("XID_Start"):
	OUT.setdefault(code, Props()).xid_start = "true"

for code in DerivedCoreProperties("XID_Continue"):
	OUT.setdefault(code, Props()).xid_continue = "true"

[stage1, stage2, stage3] = Generator.build(OUT)

S1 = """constexpr const std::array<uint16_t, %s> stage1
{
%s
};
"""
S2 = """constexpr const std::array<uint8_t, %s> stage2
{
%s
};
"""
S3 = """constexpr const std::array<props, %s> stage3
{
%s
};
"""

folder = os.path.join(__file__, "..", "..", "src", "data")

with open(os.path.join(folder, f"stage1.txt"), "w", encoding="utf-8") as f:
	f.write(S1 % (len(stage1), ",\n".join(f"\t{code}" for code in stage1) + ","))

with open(os.path.join(folder, f"stage2.txt"), "w", encoding="utf-8") as f:
	f.write(S2 % (len(stage2), ",\n".join(f"\t{code}" for code in stage2) + ","))

with open(os.path.join(folder, f"stage3.txt"), "w", encoding="utf-8") as f:
	f.write(S3 % (len(stage3), ",\n".join(f"\t{code}" for code in stage3) + ","))
