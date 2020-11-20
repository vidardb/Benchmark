#!/usr/bin/env bash

set -e

export PS4='+ [`basename ${BASH_SOURCE[0]}`:$LINENO ${FUNCNAME[0]} \D{%F %T} $$ ] '
CURDIR=$(cd "$(dirname "$0")"; pwd);
MYNAME="${0##*/}"

GITTPCH="https://github.com/vidardb/tpch-dbgen"
TPCHPATH="$CURDIR/tpch-dbgen"
TPCHCMD="$TPCHPATH/dbgen"
TPCHDAT="lineitem.tbl"

PLATFORM_PG="pg"
PLATFORM_FDW="fdw"
PLATFORM_TBAM="tbam"
PLATFORM_ENG="engine"

RET_OK=0
RET_FAIL=1

##################### function #########################
_report_err() { echo "${MYNAME}: Error: $*" >&2 ; }

if [ -t 1 ]
then
    RED="$( echo -e "\e[31m" )"
    HL_RED="$( echo -e "\e[31;1m" )"
    HL_BLUE="$( echo -e "\e[34;1m" )"

    NORMAL="$( echo -e "\e[0m" )"
fi

_hl_red()    { echo "$HL_RED""$@""$NORMAL";}
_hl_blue()   { echo "$HL_BLUE""$@""$NORMAL";}

_trace() {
    echo $(_hl_blue '  ->') "$@" >&2
}

_print_fatal() {
    echo $(_hl_red '==>') "$@" >&2
    exit $RET_FAIL
}

# shellcheck disable=SC1009
_install_tpch() {
    _trace "git clone tpch from $GITTPCH ..."
    if [ ! -d "$TPCHPATH" ]; then
        git clone "$GITTPCH" "$TPCHPATH" &> /dev/null
        # shellcheck disable=SC2181
        if [ $? -ne 0 ]; then
            _print_fatal "clone tpch-dbgen failed."
        fi
    fi

    _trace "compile source code for dbgen ..."
    cd "$TPCHPATH" && make &> /dev/null
    if [ $? -ne 0 ]; then
        _print_fatal "make tpch-dbgen failed."
    fi

    if [ -x ${TPCHCMD} ];then
        _trace "install tpch success"
    else
        _print_fatal "install tpch failed"
    fi
}

_gen_data() {
    size=$2
    if [ -z "$size" ]; then
       size=1  # default 1GB
    fi

    path=$3
    if [ -z "$path" -o "$path" = "$TPCHDAT" ]; then
        path=$TPCHPATH/$TPCHDAT
    fi

    table="L"  # lineitem
    cd $(dirname $TPCHCMD)
    if [ -e "$TPCHPATH/$TPCHDAT" ]; then
        echo "remove $TPCHPATH/$TPCHDAT"
        rm -rf "$TPCHPATH/$TPCHDAT"
    fi

    _trace "generate data for benchmark ( size: ${size}G table: $table ) ..."
    "$TPCHCMD" -f -s "$size" -T "$table"
    # important: shuffle tcph data
    shuf $TPCHPATH/$TPCHDAT -o $TPCHPATH/${TPCHDAT}.shuf
    rm -rf "$TPCHPATH/$TPCHDAT"
    mv $TPCHPATH/${TPCHDAT}.shuf $path

    if [ -e "$path" ]; then
        _trace "benchmark data path: $path"
    else
        _print_fatal "generate benchmark data failed."
    fi
}

# psql should be installed
_create_benchmark_tbl() {
    _trace "create benchmark table ..."
    if ! command -v psql > /dev/null; then
        _print_fatal "psql should be installed."
    else
        psql -h$PGHOST -p$PGPORT -U$PGUSER -d$PGDATABASE -f $CURDIR/$1
    fi
}

_run_benchmark() {
    case $2 in
        $PLATFORM_PG)
            platform="PostgreSQL"
            bench="pg_bench"
            tbl_ddl="sql/pg_tbl_ddl.sql"
            ;;
        $PLATFORM_FDW)
            platform="VidarDB"
            bench="fdw_bench"
            tbl_ddl="sql/fdw_tbl_ddl.sql"
            ;;
        $PLATFORM_TBAM)
            platform="VidarDB"
            bench="tbam_bench"
            tbl_ddl="sql/tbam_tbl_ddl.sql"
            ;;
        $PLATFORM_ENG)
            platform="VidarDB Engine"
            bench="engine_bench"
            ;;
        *)
            _print_fatal "unsupported platform: $2"
            _usage
            ;;
    esac

    _gen_data "dummy" $DATASIZE $DATASOURCE
    if [ "$platform" = "PostgreSQL" -o "$platform" = "VidarDB" ]; then
        _create_benchmark_tbl $tbl_ddl
    fi

    # run benchmark for specified platform
    _trace "starting benchmark for $platform ..."
    if [ "$DATASOURCE" = "$TPCHDAT" ]; then
        export DATASOURCE=$TPCHPATH/$TPCHDAT
    else
        export DATASOURCE=$DATASOURCE
    fi
    export PLATFORM=$PLATFORM
    export SCENARIO=$SCENARIO
    export PGHOST=$PGHOST
    export PGPORT=$PGPORT
    export PGDATABASE=$PGDATABASE
    export PGUSER=$PGUSER
    export DBPATH=$DBPATH
    $CURDIR/benchmark
}

_usage() {
    cat << USAGE
Usage: ./${MYNAME} install_tpch|gen_data|run_benchmark
Action:
    install_tpch                      Install tpch-dbgen tool.
    gen_data [size] [path]            Generate tpch data, unit is GB.
    run_benchmark [pg|fdw|engine]     Run benchmark for specified platform.
USAGE
    exit 1
}

## main ##
action=$1
case $action in
    "install_tpch" )
        _install_tpch
        ;;
    "gen_data" )
        _gen_data "$@"
        ;;
    "run_benchmark" )
        _run_benchmark "$@"
        ;;
    *)
        _usage
        ;;
esac