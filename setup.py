from distutils.core import setup

setup(
    name='npn',
    version='1.0',
    author='Xihan Li',
    author_email='xihan.li@cs.ucl.ac.uk',
    description='A boolean matcher that computes the canonical representative, which is unique for each NPN equivalence class, for a given boolean function represented by truth table.',
    url='https://github.com/snowkylin/npn',
    packages=['npn'],
    package_data={'npn': ['npn.dll', 'libnpn.so']}
)
