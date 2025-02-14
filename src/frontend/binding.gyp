{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/addon.cpp", "src/hello.cpp" ],
      "include_dirs": [
        "node_modules/node-addon-api",
        "src"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}
