#include "stdafx.h"
#include "sewindows.h"
#include "communite_with_driver.h"
#include "proc_info_list.h"
#include "lib.h"

#pragma comment(lib,"Advapi32.lib")

CCommunicateDriv	g_comm;
BOOLEAN				g_bIsDriverInited = TRUE;
CString				g_str_altitude;
CString				g_str_service_name;
CString				g_str_port_name;
CString				g_str_link_name;
CString				g_str_path;
sewin_operations	g_sewin_operation;

BOOLEAN notify_callback_func(Param& op)
{
	WCHAR	user_name[MAX_PATH] = {0};
	WCHAR	proc_path[MAX_PATH] = {0};
	WCHAR	dest_proc_path[MAX_PATH] = { 0 };
	PHIPS_RULE_NODE prule_node = &op.opdata.rule_node;

	if (op.opdata.option == OPTION_PROC_EXIT)
	{
		delete_entry_by_pid((DWORD)prule_node->sub_pid);
		return TRUE;
	}

	if (OPTION_TIME_TO_HOOK == op.opdata.option)
	{
		inject_dll_by_pid((DWORD)prule_node->sub_pid,TRUE);
		return TRUE;
	}

	switch (prule_node->major_type)
	{
	case PROC_OP:
	{
		switch (prule_node->minor_type)
		{
		case OP_PROC_KILL:
			if (g_sewin_operation.process_kill)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_kill(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_PROC_CREATE_REMOTE_THREAD:
			if (g_sewin_operation.process_create_thread)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_create_thread(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_PROC_READ_PROCESS:
			if (g_sewin_operation.process_read_mem)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_read_mem(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_PROC_WRITE_PROCESS:
			if (g_sewin_operation.process_write_mem)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_write_mem(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_PROC_CREATE_PROCESS:
			if (g_sewin_operation.process_create)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.process_create(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_PROC_CHANGE_VM:
			if (g_sewin_operation.process_set_mem_attr)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_set_mem_attr(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_PROC_SUSPEND_RESUME:
			if (g_sewin_operation.process_susresume)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_susresume(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_PROC_DUPHANDLE:
			if (g_sewin_operation.process_dup_handlle)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.process_dup_handlle(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_THREAD_KILL:
			if (g_sewin_operation.thread_kill)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.thread_kill(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_THREAD_GET_CONTEXT:
			if (g_sewin_operation.thread_get_context)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.thread_get_context(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_THREAD_SET_CONTEXT:
			if (g_sewin_operation.thread_set_context)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.thread_set_context(user_name, proc_path, dest_proc_path);
			}
			break;
		case OP_THREAD_SUSPEND_RESUME:
			if (g_sewin_operation.thread_susresume)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				get_proc_path_by_pid((DWORD)prule_node->obj_pid, dest_proc_path);
				return g_sewin_operation.thread_susresume(user_name, proc_path, dest_proc_path);
			}
			break;
		default:
			break;
		}
		break;
	}
	case REG_OP:
	{
		switch (prule_node->minor_type)
		{
		case OP_REG_READ:
			if (g_sewin_operation.reg_read_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_read_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_DELETE_VALUE_KEY:
			if (g_sewin_operation.reg_delete_value)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_delete_value(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_CREATE_KEY:
			if (g_sewin_operation.reg_create_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_create_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_SET_VALUE_KEY:
			if (g_sewin_operation.reg_set_value)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_set_value(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer(), prule_node->new_name);
			}
			break;
		case OP_REG_RENAME:
			if (g_sewin_operation.reg_rename_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_rename_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer(), prule_node->new_name);
			}
			break;
		case OP_REG_DELETE_KEY:
			if (g_sewin_operation.reg_delete_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_delete_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_SAVE:
			if (g_sewin_operation.reg_save_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_save_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_RESTORE:
			if (g_sewin_operation.reg_restore_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_restore_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_REPLACE:
			if (g_sewin_operation.reg_replace)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_replace(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_LOAD:
			if (g_sewin_operation.reg_load_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_load_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;
		case OP_REG_UNLOAD:
			if (g_sewin_operation.reg_unload_key)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
				return g_sewin_operation.reg_unload_key(user_name, proc_path, trans_reg_path(prule_node->des_path).GetBuffer());
			}
			break;

		default:
			break;
		}
		break;
	}
	case SERVICE_OP:
	{
		switch (prule_node->minor_type)
		{
		case OP_SERVICE_CREATE:
			if (g_sewin_operation.service_create)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				return g_sewin_operation.service_create(user_name, proc_path, prule_node->service_name, prule_node->des_path);
			}
			break;
		case OP_SERVICE_DELETE:
			if (g_sewin_operation.service_delete)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				return g_sewin_operation.service_delete(user_name, proc_path, prule_node->service_name);
			}
			break;
		case OP_SERVICE_CHANGE:
			if (g_sewin_operation.service_change)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				return g_sewin_operation.service_change(user_name, proc_path, prule_node->service_name);
			}
			break;
		case OP_SERVICE_DRIVER:
			if (g_sewin_operation.driver_load)
			{
				get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
				return g_sewin_operation.driver_load(user_name, proc_path, prule_node->service_name, prule_node->des_path);
			}
			break;
		default:
			break;
		}
		break;
	}
		/*BOOLEAN(*disk_read)             (WCHAR *user_name, WCHAR *process, WCHAR *dir_path);
		BOOLEAN(*disk_write)            (WCHAR *user_name, WCHAR *process, WCHAR *dir_path);
		BOOLEAN(*disk_format)           (WCHAR *user_name, WCHAR *process, WCHAR *dir_path);*/
	case FILE_OP:
	{
		if (prule_node->is_dir)
		{
			switch (prule_node->minor_type)
			{
			case DISK_READ_XX:
				if (g_sewin_operation.disk_read)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.disk_read(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case DISK_WRITE_XX:
				if (g_sewin_operation.disk_write)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
					return g_sewin_operation.disk_write(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case DISK_FORMAT_XX:
				if (g_sewin_operation.disk_format)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
					return g_sewin_operation.disk_format(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_DEL_XX:
				if (g_sewin_operation.dir_unlink)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name, proc_path);
					return g_sewin_operation.dir_unlink(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_RENAME_XX:
				if (g_sewin_operation.dir_rename)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.dir_rename(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer(), trans_file_path(prule_node->new_name).GetBuffer());
				}
				break;
			case FILE_CREATE_XX:
				if (g_sewin_operation.dir_create)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.dir_create(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_SETINFO_XX:
				if (g_sewin_operation.dir_set_attr)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.dir_set_attr(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer(), &prule_node->fbi);
				}
				break;
			default:
				break;
			}
		}
		else
		{
			switch (prule_node->minor_type)
			{
			case FILE_READ_DATA_XX:
				if (g_sewin_operation.file_read)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_read(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_WRITE_DATA_XX:
				if (g_sewin_operation.file_write)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_write(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_DEL_XX:
				if (g_sewin_operation.file_unlink)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_unlink(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_RENAME_XX:
				if (g_sewin_operation.file_rename)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_rename(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer(), trans_file_path(prule_node->new_name).GetBuffer());
				}
				break;
			case FILE_CREATE_XX:
				if (g_sewin_operation.file_create)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_create(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			case FILE_SETINFO_XX:
				if (g_sewin_operation.file_set_attr)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_set_attr(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer(), &prule_node->fbi);
				}
				break;
			case FILE_EXECUTE_XX:
				if (g_sewin_operation.file_execute)
				{
					get_proc_info_by_pid(prule_node->sub_pid, user_name,proc_path);
					return g_sewin_operation.file_execute(user_name, proc_path, trans_file_path(prule_node->des_path).GetBuffer());
				}
				break;
			default:
				break;
			}
		}
		break;
	}
	default:
		break;
	}
	return TRUE;
}


void sewin_uninit(void)
{
	set_privilege(SE_DEBUG_NAME, FALSE);
	g_comm.RemoveHook();
	g_comm.PermitUnload();
	g_comm.CloseComplete();
	g_comm.OnExitProcess(g_str_service_name.GetBuffer());
	uninit_user_list();
}

SEWINDOWS_API BOOLEAN sewin_init(void)
{
	ULONG		top_altitude;
	WCHAR		windows_directory[MAX_PATH];
	if (!init_lib())
	{
		return FALSE;	 
	}
	init_user_list();

	RtlZeroMemory(&g_sewin_operation, sizeof(g_sewin_operation));
	top_altitude = get_the_top_altitude();

	if (top_altitude == 0)
	{
		return FALSE;
	}
	set_privilege(SE_DEBUG_NAME, TRUE);
	g_str_altitude.Format(_T("%d"), top_altitude);
	/*g_str_service_name.Format(_T("%08d"), 100);
	g_str_port_name.Format(_T("%08d"), 100);
	g_str_link_name.Format(_T("%08d"), 100);*/
	g_str_path = get_module_path();
	if (g_str_path.GetAt(g_str_path.GetLength()-1) != _T('\\'))
	{
		g_str_path += _T("\\");
	}
	g_str_path += DRIVERNAME;
	g_str_service_name = SERVICENAME ;
	g_str_port_name = HIPSPORTNAME ;
	g_str_link_name = LINKNAME ;
	g_comm.SetNotify(notify_callback_func);
	if (!g_comm.InitDriver(g_str_service_name.GetBuffer(), g_str_path.GetBuffer(), g_str_altitude.GetBuffer(), g_str_port_name.GetBuffer(), g_str_link_name.GetBuffer()))
	{
		MessageBox(NULL,g_comm.m_errStr,NULL,0);
		g_bIsDriverInited = FALSE;
		return FALSE;
	}

//#ifdef	DEBUG
//	DebugBreak();
//#endif

	bulid_p2u_map();

	if (!get_windows_directory(windows_directory))
	{
		return FALSE;
	}
	if (!g_comm.TransferSysroot(windows_directory))
	{
		return FALSE;
	}//IOCTL_TRANSFER_INJECT_DLL

	WCHAR temp_path[MAX_PATH];
	if (!GetWindowsDirectoryW(temp_path, MAX_PATH))
	{
		return FALSE;
	}
	if (temp_path[wcslen(temp_path)-1] != L'\\')
	{
		wcscat_s(temp_path, MAX_PATH, L"\\");
	}
	wcscat_s(temp_path, MAX_PATH, L"system32\\");
	wcscat_s(temp_path, MAX_PATH, MONDLLNAME);

	CString path = get_module_path();
	if (path.GetAt(path.GetLength() - 1) != _T('\\'))
	{
		path += _T("\\");
	}
	path += MONDLLNAME;

	CopyFile(path, temp_path, FALSE);

	if (!g_comm.TransferInjectDll(temp_path))
	{
		return FALSE;
	}
	g_bIsDriverInited = g_comm.StopUnload();
	return g_bIsDriverInited;
}


SEWINDOWS_API BOOLEAN sewin_setoption(int mode, int type)
{
	BOOLEAN bret = FALSE;
	if (mode != SEWIN_MODE_INTERCEPT && mode != SEWIN_MODE_NOTIFY)
	{
		return FALSE;
	}

	if (mode == SEWIN_MODE_INTERCEPT)
	{
		bret = g_comm.SetMode(FALSE);
	}
	else
	{
		bret = g_comm.SetMode(TRUE);
	}

	if (!bret)
	{
		return bret;
	}

	if (type & SEWIN_TYPE_FILE)
	{
		bret = g_comm.StartFileMon(TRUE);
	}
	else
	{
		bret = g_comm.StartFileMon(FALSE);
	}
	if (!bret)
	{
		return bret;
	}
	if (type & SEWIN_TYPE_PROC)
	{
		bret = g_comm.StartProcMon(TRUE);
	}
	else
	{
		bret = g_comm.StartProcMon(FALSE);
	}
	if (!bret)
	{
		return bret;
	}
	if (type & SEWIN_TYPE_REG)
	{
		bret = g_comm.StartRegMon(TRUE);
	}
	else
	{
		bret = g_comm.StartRegMon(FALSE);
	}
	if (!bret)
	{
		return bret;
	}

	if (type & SEWIN_TYPE_SCVDRV)
	{
		bret = g_comm.StartSvcMon(TRUE);
	}
	else
	{
		bret = g_comm.StartSvcMon(FALSE);
	}
	if (!bret)
	{
		return bret;
	}

	return TRUE;
}
SEWINDOWS_API BOOLEAN sewin_register_opt(struct sewin_operations *ops)
{
	if (ops == NULL)
	{
		return FALSE;
	}
	g_sewin_operation = *ops;
	return TRUE;
}
