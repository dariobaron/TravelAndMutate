

def splitInput(string):
	names = []
	if "," in string:
		string = string.split(",")
	if not isinstance(string, list):
		string = [string]
	for el in string:
		if ":" in el:
			start,finish = el.split(":")
			names.extend(range(int(start),int(finish)))
		else:
			names.append(int(el))
	names = [str(name) for name in names]
	return names