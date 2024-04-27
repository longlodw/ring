with import <nixpkgs> {};
mkShell {
  packages = with pkgs; [
    nodejs
    unzip
    bear
    gnumake
  ];
  shellHook = '''';
  nativeBuildInputs = with pkgs; [];
  buildInputs = with pkgs; [];
}
