script_folder="/home/mikhail/github/LampyEngine/Engine/ThirdParty"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-debug-x86_64.sh"
for v in LD_LIBRARY_PATH DYLD_LIBRARY_PATH ALSA_CONFIG_DIR
do
    is_defined="true"
    value=$(printenv $v) || is_defined="" || true
    if [ -n "$value" ] || [ -n "$is_defined" ]
    then
        echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-debug-x86_64.sh"
    else
        echo unset $v >> "$script_folder/deactivate_conanrunenv-debug-x86_64.sh"
    fi
done


export LD_LIBRARY_PATH="/home/mikhail/.conan2/p/b/vulka1275014f80218/p/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="/home/mikhail/.conan2/p/b/vulka1275014f80218/p/lib:$DYLD_LIBRARY_PATH"
export ALSA_CONFIG_DIR="/home/mikhail/.conan2/p/b/libal021646ebc23d7/p/res/alsa"