#include "main.h"
#include "processmon.h"
#include "common.h"
#include "lpc.h"
#include "regmon.h"
#include "filemon.h"
#include <strsafe.h>


#if (NTDDI_VERSION >= NTDDI_VISTA)

PVOID						g_proc_callback_handle = NULL;
OB_CALLBACK_REGISTRATION	g_proc_callback = { 0 };
OB_OPERATION_REGISTRATION	g_operation_registration[2] = { { 0 }, { 0 } };
static BOOLEAN				g_bSetCreateProcessNotify = FALSE;

OB_PREOP_CALLBACK_STATUS pre_procopration_callback( PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
{
	HIPS_RULE_NODE	Pi;
	PWCHAR			pPath = NULL;
	WCHAR			tmpPath[MAXPATHLEN] = { 0 };
	WCHAR			wszLongName[MAXPATHLEN] = { 0 };
	HANDLE			target_pid = NULL;
	ACCESS_MASK		OriginalDesiredAccess = 0;
	PACCESS_MASK	DesiredAccess = NULL;

	if (pOperationInformation->KernelHandle == TRUE || g_is_proc_run == FALSE)
	{
		return OB_PREOP_SUCCESS;
	}

	if (pOperationInformation->ObjectType == *PsThreadType)
	{
		return OB_PREOP_SUCCESS;
	}
	else if (pOperationInformation->ObjectType == *PsProcessType)
	{
		target_pid = PsGetProcessId((PEPROCESS)pOperationInformation->Object);
	}
	else
	{
		return OB_PREOP_SUCCESS;
	}

	if ((PsGetCurrentProcessId() == target_pid))
	{
		return OB_PREOP_SUCCESS;
	}

	switch (pOperationInformation->Operation) 
	{
	case OB_OPERATION_HANDLE_CREATE:
		DesiredAccess = &pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess;
		OriginalDesiredAccess = pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;
		break;
	case OB_OPERATION_HANDLE_DUPLICATE:
		DesiredAccess = &pOperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess;
		OriginalDesiredAccess = pOperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess;
		break;
	default:
		return OB_PREOP_SUCCESS;
	}

	RtlZeroMemory(&Pi, sizeof(HIPS_RULE_NODE));
	Pi.major_type = PROC_OP;
	Pi.sub_pid = PsGetCurrentProcessId();		
	
	
	pPath = get_proc_name_by_pid(Pi.sub_pid, tmpPath);
	if (NULL == pPath )
	{
		return OB_PREOP_SUCCESS;
	}

	if (is_short_name_path(pPath))
	{
		convert_short_name_to_long(wszLongName, pPath, sizeof(WCHAR)*MAXPATHLEN);
		RtlCopyMemory(pPath, wszLongName, sizeof(WCHAR)*MAXPATHLEN);
	}

	if (!get_dos_name(pPath, Pi.src_path))
	{
		StringCbCopyW(Pi.src_path, sizeof(Pi.src_path), pPath);
	}


	Pi.obj_pid = target_pid;		
	
	pPath = get_proc_name_by_pid(Pi.obj_pid,tmpPath);
	if (NULL == pPath)
	{
		return OB_PREOP_SUCCESS;
	}

	if (is_short_name_path(pPath))
	{
		convert_short_name_to_long(wszLongName, pPath, sizeof(WCHAR)*MAXPATHLEN);
		RtlCopyMemory(pPath, wszLongName, sizeof(WCHAR)*MAXPATHLEN);
	}

	if (!get_dos_name(pPath, Pi.des_path))
	{
		StringCbCopyW(Pi.des_path, sizeof(Pi.des_path), pPath);
	}

	if (pOperationInformation->ObjectType == *PsProcessType)
	{
		if ((OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE)
		{
			//	ɱ������
			Pi.minor_type = OP_PROC_KILL;
			if (rule_match(&Pi) == FALSE)
			{
				*DesiredAccess &= ~PROCESS_TERMINATE;
			}
		}
		if ((OriginalDesiredAccess & PROCESS_CREATE_THREAD) == PROCESS_CREATE_THREAD)
		{	//	Զ���̴߳���
			Pi.minor_type = OP_PROC_CREATE_REMOTE_THREAD;
			if (rule_match(&Pi) == FALSE)
			{
				*DesiredAccess &= ~PROCESS_CREATE_THREAD;
			}
		}
		if ((OriginalDesiredAccess & PROCESS_VM_OPERATION) == PROCESS_VM_OPERATION)
		{	//	�޸��ڴ�����
			Pi.minor_type = OP_PROC_CHANGE_VM;
			if (rule_match(&Pi) == FALSE)
			{
				*DesiredAccess &= ~PROCESS_VM_OPERATION;
			}
		}
		if ((OriginalDesiredAccess & PROCESS_VM_READ) == PROCESS_VM_READ)
		{	//	���ڴ�
			Pi.minor_type = OP_PROC_READ_PROCESS;
			if (rule_match(&Pi) == FALSE)
			{
				*DesiredAccess &= ~PROCESS_VM_READ;
			}
		}
		if ((OriginalDesiredAccess & PROCESS_VM_WRITE) == PROCESS_VM_WRITE)
		{	//	д�ڴ�
			Pi.minor_type = OP_PROC_WRITE_PROCESS;
			if (rule_match(&Pi) == FALSE)
			{
				*DesiredAccess &= ~PROCESS_VM_WRITE;
			}
		}
	}
	return OB_PREOP_SUCCESS;
}

VOID create_process_notity_routine( PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	PWCHAR pPath = NULL;
	HIPS_RULE_NODE Pi;
	WCHAR	tmpPath[MAXPATHLEN] = { 0 };
	WCHAR	wszLongName[MAXPATHLEN] = { 0 };
	RtlZeroMemory(&Pi, sizeof(HIPS_RULE_NODE));
	Pi.major_type = PROC_OP;

	if (g_is_proc_run == FALSE)
	{
		return;
	}

	if (CreateInfo != NULL)
	{
		Pi.sub_pid = CreateInfo->ParentProcessId;
		Pi.obj_pid = ProcessId;
		Pi.minor_type = OP_PROC_CREATE_PROCESS;
		pPath = get_proc_name_by_pid(Pi.sub_pid, tmpPath);
		if (NULL == pPath)
		{
			return ;
		}

		if (is_short_name_path(pPath))
		{
			convert_short_name_to_long(wszLongName, pPath, sizeof(WCHAR)*MAXPATHLEN);
			StringCbCopyW(pPath, sizeof(WCHAR)*MAXPATHLEN, wszLongName);
		}

		if (!get_dos_name(pPath, Pi.src_path))
		{
			StringCbCopyW(Pi.src_path, sizeof(Pi.src_path), pPath);
		}

		StringCbCopyNW(tmpPath, sizeof(tmpPath), CreateInfo->ImageFileName->Buffer, CreateInfo->ImageFileName->Length);

		if (is_short_name_path(tmpPath))
		{
			convert_short_name_to_long(wszLongName, tmpPath, sizeof(WCHAR)*MAXPATHLEN);
			StringCbCopyW(tmpPath, sizeof(WCHAR)*MAXPATHLEN,wszLongName);
		}

		if (!get_dos_name(tmpPath, Pi.des_path))
		{
			StringCbCopyW(Pi.des_path, sizeof(Pi.des_path), tmpPath);
		}
		if (rule_match(&Pi) == FALSE)
		{
			CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		if (g_currentPid == ProcessId)
		{
			g_is_file_run = FALSE;
			g_is_proc_run = FALSE;
			g_is_reg_run = FALSE;

		}
	}
}

NTSTATUS sw_init_procss(PDRIVER_OBJECT pDriverObj)
{
	NTSTATUS					Status = STATUS_SUCCESS;
	UNICODE_STRING				altitude = { 0 };
	WCHAR						szBuffer[20];
	ULONGLONG					ul_altitude = 1000;
	Status = PsSetCreateProcessNotifyRoutineEx(create_process_notity_routine,FALSE);
	if (NT_SUCCESS(Status))
	{
		g_bSetCreateProcessNotify = TRUE;

		g_operation_registration[0].ObjectType = PsProcessType;
		g_operation_registration[0].Operations |= OB_OPERATION_HANDLE_CREATE;
		g_operation_registration[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
		g_operation_registration[0].PreOperation = pre_procopration_callback;

		g_operation_registration[1].ObjectType = PsThreadType;
		g_operation_registration[1].Operations |= OB_OPERATION_HANDLE_CREATE;
		g_operation_registration[1].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
		g_operation_registration[1].PreOperation = pre_procopration_callback;
		g_proc_callback.Version = OB_FLT_REGISTRATION_VERSION;
		g_proc_callback.OperationRegistrationCount = 2;
		g_proc_callback.RegistrationContext = NULL;
		g_proc_callback.OperationRegistration = g_operation_registration;

try_again:
		RtlZeroMemory(szBuffer, sizeof(szBuffer));
		RtlInitEmptyUnicodeString(&altitude, szBuffer, 20 * sizeof(WCHAR));
		RtlInt64ToUnicodeString(ul_altitude, 10, &altitude);
		g_proc_callback.Altitude = altitude;
		Status = ObRegisterCallbacks(
			&g_proc_callback,
			&g_proc_callback_handle       
			);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}

		if (STATUS_FLT_INSTANCE_ALTITUDE_COLLISION == Status)
		{
			ul_altitude++;
			if (ul_altitude<100000)
			{
				goto try_again;
			}
		}
		PsSetCreateProcessNotifyRoutineEx(create_process_notity_routine, TRUE);
		g_bSetCreateProcessNotify = FALSE;
	}
	return Status;
}

NTSTATUS sw_uninit_procss(PDRIVER_OBJECT pDriverObj)
{
	NTSTATUS Status = STATUS_SUCCESS;
	if (g_bSetCreateProcessNotify)
	{
		Status = PsSetCreateProcessNotifyRoutineEx(create_process_notity_routine, TRUE);
		g_bSetCreateProcessNotify = FALSE;
	}

	if (g_proc_callback_handle)
	{
		ObUnRegisterCallbacks(g_proc_callback_handle);
		g_proc_callback_handle = NULL;
	}
	return Status;
}
#endif