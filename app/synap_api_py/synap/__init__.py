import importlib.util
from pathlib import Path

# TODO: this is a bit hacky, should be replaced with a more robust solution

so_file = next(Path(__file__).parent.glob("*.so"))
spec = importlib.util.spec_from_file_location("synap", str(so_file.resolve()))
synap = importlib.util.module_from_spec(spec)
spec.loader.exec_module(synap)

globals().update({name: getattr(synap, name) for name in dir(synap) if not name.startswith("_")})
