#!/usr/bin/env bash

set -e

export PS4='+ [`basename ${BASH_SOURCE[0]}`:$LINENO ${FUNCNAME[0]} \D{%F %T} $$ ] '
CURDIR=$(cd "$(dirname "$0")"; pwd);
MYNAME="${0##*/}"

GITTPCH="https://github.com/vidardb/tpch-dbgen"
TPCHPATH="$CURDIR/tpch-dbgen"
TPCHCMD="$TPCHPATH/dbgen"

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
    cd "$CURDIR/tpch-dbgen" && make &> /dev/null
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
       size=1
    fi

    table="L"  # lineitem
    cd $(dirname $TPCHCMD)
    if [ -e "$TPCHPATH/lineitem.tbl" ]; then
        rm -f "$TPCHPATH/lineitem.tbl"
    fi

    _trace "generate data for benchmark ( size: ${size}G table: $table ) ..."
    "$TPCHCMD" -f -s "$size" -T "$table"
    _trace "benchmark data path: $TPCHPATH/lineitem.tbl"
}

# psql should be installed
_create_benchmark_tbl() {
    _trace "create benchmark table ..."
    if ! command -v psql > /dev/null; then
        _print_fatal "psql should be installed"
    else
        psql -h$PGHOST -p$PGPORT -U$PGUSER -d$PGDATABASE -f $CURDIR/$1
    fi
}

_run_benchmark() {
    platform="VidarDB"
    bench="fdw_bench"
    tbl_ddl="sql/fdw_tbl_ddl.sql"
    if [ "$2" = "pg" ]; then
        platform="PostgreSQL"
        bench="pg_bench"
        tbl_ddl="sql/pg_tbl_ddl.sql"
    fi

    _gen_data $DATASIZE
    _create_benchmark_tbl $tbl_ddl

    # run benchmark
    _trace "starting benchmark for $platform ..."
    export DATASOURCE=$TPCHPATH/$DATASOURCE
    export PGHOST=$PGHOST
    export PGPORT=$PGPORT
	export PGDATABASE=$PGDATABASE
    export PGUSER=$PGUSER
    $CURDIR/$bench
}

_usage() {
    cat << USAGE
Usage: bash ${MYNAME} install_tpch|gen_data|run_benchmark
Action:
    install_tpch                      Install tpch-dbgen.
    gen_data [size_factor]            Generate tpch data.
    run_benchmark [pg|fdw]            Run benchmark.
USAGE
    exit 1
}

## main
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