import ../libalbinos/albinos

proc createCfg*(configName: cstring): (ptr Config, ReturnedValue) =
  var cfg: ptr Config
  var res = createConfig(configName, addr cfg)
  return (cfg, res)